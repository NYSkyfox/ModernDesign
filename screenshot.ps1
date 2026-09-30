# ModernDesign CI 截图脚本（基于 FluentZero v4 方案，全屏截图）
# 关键点：
#   - 用 EnumWindows 按类名 "ModernDesignAppWindow" 找窗口（最可靠）
#   - Runner 上 DWM 不真模糊（无 GPU，走 WARP），窗口后方是桌面
#   - 全屏截图：截取整个虚拟桌面，便于看清窗口在桌面的位置与内容
$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName System.Windows.Forms

# exe 可能在多个位置（CMake 输出目录），逐一查找
$candidates = @(
    "build\bin\DemoAppDemo.exe",
    "build\DemoAppDemo.exe",
    "build\DemoApp\DemoAppDemo.exe"
)
$exe = $null
foreach ($c in $candidates) {
    $p = Join-Path $PWD $c
    if (Test-Path $p) { $exe = $p; break }
}
if (-not $exe) {
    # 兜底：递归找任意 DemoAppDemo.exe
    $found = Get-ChildItem -Path build -Recurse -Filter "DemoAppDemo.exe" -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($found) { $exe = $found.FullName }
}
if (-not $exe) { throw "exe not found under build/" }
Write-Host "exe: $exe"

$proc = Start-Process -FilePath $exe -WorkingDirectory $PWD -PassThru

Add-Type @"
using System;
using System.Runtime.InteropServices;
using System.Text;
public class MdW {
    public static IntPtr found = IntPtr.Zero;
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int l, t, r, b; }
    [DllImport("user32.dll", EntryPoint="GetClassNameW", CharSet=CharSet.Unicode)]
    public static extern int GetClassName(IntPtr h, StringBuilder s, int max);
    [DllImport("user32.dll", EntryPoint="GetWindowTextW", CharSet=CharSet.Unicode)]
    public static extern int GetWindowText(IntPtr h, StringBuilder s, int max);
    public delegate bool EnumCB(IntPtr h, IntPtr l);
    [DllImport("user32.dll")] public static extern bool EnumWindows(EnumCB cb, IntPtr l);
    [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int cmd);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
    public static IntPtr FindByClass(string cls) {
        found = IntPtr.Zero;
        EnumWindows((h, l) => {
            var sb = new StringBuilder(256);
            GetClassName(h, sb, 256);
            if (sb.ToString() == cls) { found = h; return false; }
            return true;
        }, IntPtr.Zero);
        return found;
    }
    [DllImport("user32.dll")] public static extern bool IsWindow(IntPtr h);
    public static void MinimizeExcept(string keepCls) {
        int minimized = 0;
        EnumWindows((h, l) => {
            if (!IsWindow(h) || !IsWindowVisible(h)) return true;
            var cb = new StringBuilder(256); GetClassName(h, cb, 256);
            string cls = cb.ToString();
            if (cls == keepCls) return true;
            if (cls == "Shell_TrayWnd" || cls == "Progman" || cls == "WorkerW"
                || cls == "Shell_SecondaryTrayWnd" || cls == "ShellDesktop"
                || cls == "Windows.UI.Core.CoreWindow") return true;
            ShowWindow(h, 6);
            minimized++;
            return true;
        }, IntPtr.Zero);
        Console.WriteLine("    MinimizeExcept(" + keepCls + "): " + minimized + " minimized");
    }
    public static void Dump() {
        EnumWindows((h, l) => {
            if (IsWindowVisible(h)) {
                var c = new StringBuilder(256); GetClassName(h, c, 256);
                var t = new StringBuilder(256); GetWindowText(h, t, 256);
                Console.WriteLine("WIN [" + c + "] title=" + t);
            }
            return true;
        }, IntPtr.Zero);
    }
}
"@

# 轮询等待窗口出现（最多 12 秒）
$wndCls = "ModernDesignAppWindow"
$hwnd = [IntPtr]::Zero
for ($i = 0; $i -lt 24; $i++) {
    Start-Sleep -Milliseconds 500
    $hwnd = [MdW]::FindByClass($wndCls)
    if ($hwnd -ne [IntPtr]::Zero) { break }
}

$shotPath = Join-Path $PWD "screenshot.png"

if ($hwnd -eq [IntPtr]::Zero) {
    Write-Host "!!! 主窗口未出现 —— 打印诊断:"
    $screen = [System.Windows.Forms.SystemInformation]::VirtualScreen
    $bmp = New-Object System.Drawing.Bitmap($screen.Width, $screen.Height)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.CopyFromScreen($screen.X, $screen.Y, 0, 0, $bmp.Size)
    $g.Dispose()
    $bmp.Save($shotPath, [System.Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
    [MdW]::Dump()
    Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
    throw "ModernDesign 主窗口未创建（可能崩溃或未显示）"
}

Write-Host "找到窗口 handle=$hwnd"
# 1) 最小化除 ModernDesign 外的所有顶层窗口
[MdW]::MinimizeExcept($wndCls)
Start-Sleep -Milliseconds 600
# 2) 最大化 ModernDesign（铺满屏幕：内容区更大）
[MdW]::ShowWindow($hwnd, 3) | Out-Null       # SW_MAXIMIZE = 3
[MdW]::SetForegroundWindow($hwnd) | Out-Null
Start-Sleep -Milliseconds 1200                # 等最大化重绘 + DWM 稳定

$rect = New-Object MdW+RECT
[MdW]::GetWindowRect($hwnd, [ref]$rect) | Out-Null
$w = $rect.r - $rect.l; $h = $rect.b - $rect.t
Write-Host "窗口矩形: ($($rect.l),$($rect.t)) ${w}x${h}"

# 全屏截图：截取整个虚拟桌面
$screen = [System.Windows.Forms.SystemInformation]::VirtualScreen
$sw = $screen.Width; $sh = $screen.Height
Write-Host "虚拟屏幕: ($($screen.X),$($screen.Y)) ${sw}x${sh}"
if ($sw -lt 20 -or $sh -lt 20) {
    $sw = $w; $sh = $h
    $bmp = New-Object System.Drawing.Bitmap($sw, $sh)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.CopyFromScreen($rect.l, $rect.t, 0, 0, (New-Object System.Drawing.Size($sw, $sh)))
    $g.Dispose()
} else {
    $bmp = New-Object System.Drawing.Bitmap($sw, $sh)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.CopyFromScreen($screen.X, $screen.Y, 0, 0, (New-Object System.Drawing.Size($sw, $sh)))
    $g.Dispose()
}
$bmp.Save($shotPath, [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()
Write-Host ("已保存全屏截图: {0} ({1} KB)" -f $shotPath, [math]::Round((Get-Item $shotPath).Length/1KB, 1))

Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue