# ModernDesign CI 截图脚本
# 关键点：
#   - D2D + WS_EX_NOREDIRECTIONBITMAP 窗口的内容不经过 GDI，
#     用 CopyFromScreen / BitBlt 抓不到 → 会露出桌面。
#   - 改用 PrintWindow(hwnd, dc, PW_RENDERFULLCONTENT=2)：
#     官方专为 D2D/D3D/DirectComposition 窗口设计的抓取方式。
#   - 内嵌 C# 只做 user32 的 P/Invoke（不引用 System.Drawing，避免 Add-Type 缺程序集）；
#     位图创建 / 取 DC / 保存 全部在 PowerShell 侧用 [System.Drawing.*] 完成。
param(
    # 初始导航模式（传给 App 的 MODERNDESIGN_NAV_MODE）：""/left | compact | minimal | top
    [string]$NavMode = "",
    # 输出文件名
    [string]$OutName = "screenshot.png",
    # 启动即弹出 ContentDialog（用于给弹窗出图）
    [switch]$ShowDialog,
    # 启动即弹出浮出层（flyout | menu | tooltip）
    [string]$Popup = ""
)
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
    $found = Get-ChildItem -Path build -Recurse -Filter "DemoAppDemo.exe" -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($found) { $exe = $found.FullName }
}
if (-not $exe) { throw "exe not found under build/" }
Write-Host "exe: $exe"

# 关闭 WS_EX_NOREDIRECTIONBITMAP，让 D2D 走 GDI 重定向路径 → 截图可捕获客户区
# （CI 是 WARP 无 GPU，Acrylic blur 本就不生效，关闭无损失）
$env:MODERNDESIGN_NO_NOREDIRECT = "1"
# 初始导航模式（用于逐模式出图：Left / LeftCompact / LeftMinimal / Top）
if ($NavMode -ne "") { $env:MODERNDESIGN_NAV_MODE = $NavMode }
else { Remove-Item Env:\MODERNDESIGN_NAV_MODE -ErrorAction SilentlyContinue }
# 启动即弹出弹窗（用于给 ContentDialog 出图）
if ($ShowDialog) { $env:MODERNDESIGN_SHOW_DIALOG = "1" }
else { Remove-Item Env:\MODERNDESIGN_SHOW_DIALOG -ErrorAction SilentlyContinue }
# 启动即弹出浮出层（Flyout / MenuFlyout / ToolTip）
Remove-Item Env:\MODERNDESIGN_SHOW_FLYOUT -ErrorAction SilentlyContinue
Remove-Item Env:\MODERNDESIGN_SHOW_MENU -ErrorAction SilentlyContinue
Remove-Item Env:\MODERNDESIGN_SHOW_TOOLTIP -ErrorAction SilentlyContinue
switch ($Popup.ToLower()) {
    "flyout"  { $env:MODERNDESIGN_SHOW_FLYOUT = "1" }
    "menu"    { $env:MODERNDESIGN_SHOW_MENU = "1" }
    "tooltip" { $env:MODERNDESIGN_SHOW_TOOLTIP = "1" }
}

$proc = Start-Process -FilePath $exe -WorkingDirectory $PWD -PassThru

# ---- 内嵌 C#：仅 user32 P/Invoke（不依赖 System.Drawing）----
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
    [DllImport("user32.dll")] public static extern bool IsWindow(IntPtr h);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int cmd);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
    [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);

    // PW_RENDERFULLCONTENT：抓取 D2D/D3D/DirectComposition 内容
    public const uint PW_RENDERFULLCONTENT = 2;
    [DllImport("user32.dll")]
    public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);

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

$shotPath = Join-Path $PWD $OutName

if ($hwnd -eq [IntPtr]::Zero) {
    Write-Host "!!! 主窗口未出现 —— 打印诊断:"
    [MdW]::Dump()
    Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
    throw "ModernDesign 主窗口未创建（可能崩溃或未显示）"
}
Write-Host "找到窗口 handle=$hwnd"

# 1) 最小化其它顶层窗口
[MdW]::MinimizeExcept($wndCls)
Start-Sleep -Milliseconds 600
# 2) 最大化并置前，给足渲染时间
[MdW]::ShowWindow($hwnd, 3) | Out-Null       # SW_MAXIMIZE = 3
[MdW]::SetForegroundWindow($hwnd) | Out-Null
Start-Sleep -Milliseconds 1500                # 等最大化重绘 + DWM 稳定

# 取窗口尺寸
$rect = New-Object MdW+RECT
[MdW]::GetWindowRect($hwnd, [ref]$rect) | Out-Null
$w = $rect.r - $rect.l; $h = $rect.b - $rect.t
Write-Host "窗口矩形: ($($rect.l),$($rect.t)) ${w}x${h}"
if ($w -lt 20 -or $h -lt 20) { $w = 1024; $h = 768 }

# 3) 把鼠标移离控件区：CI runner 的光标默认停在屏幕正中，
#    可能恰好压在某个控件上 → 截图里出现"随机 hover"假象。
#    移到窗口右下角空白处，触发 WM_MOUSEMOVE/WM_MOUSELEAVE 让 hover 复位。
[MdW]::SetCursorPos($rect.r - 40, $rect.b - 40) | Out-Null
Start-Sleep -Milliseconds 600

# ---- 主方式：截整个虚拟屏幕（含任务栏）----
# App 在 CI 里设了 MODERNDESIGN_NO_NOREDIRECT=1，关闭 WS_EX_NOREDIRECTIONBITMAP，
# D2D 内容走 GDI 重定向路径 → CopyFromScreen 可以直接抓到。
# 全屏截图能看到窗口在桌面上的真实位置与尺寸（最大化时不会有窗口矩形多出的 8px 黑边）。
$captured = $false
$screen = [System.Windows.Forms.SystemInformation]::VirtualScreen
Write-Host "虚拟屏幕: ($($screen.X),$($screen.Y))  $($screen.Width)x$($screen.Height)"
try {
    $bmp = New-Object System.Drawing.Bitmap($screen.Width, $screen.Height)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.CopyFromScreen($screen.X, $screen.Y, 0, 0, $bmp.Size)
    $g.Dispose()

    # 全黑检测（无交互桌面 / 会话锁定时 CopyFromScreen 会得到全黑图）
    $sum = 0.0; $n = 0
    for ($yy = 0; $yy -lt $screen.Height; $yy += 16) {
        for ($xx = 0; $xx -lt $screen.Width; $xx += 16) {
            $c = $bmp.GetPixel($xx, $yy)
            $sum += ($c.R + $c.G + $c.B); $n++
        }
    }
    $avg = if ($n -gt 0) { $sum / (3.0 * $n) } else { 0.0 }
    Write-Host ("全屏亮度均值 = {0:N1}" -f $avg)

    if ($avg -lt 5.0) {
        Write-Host "全屏截图疑似全黑，改用回退方案"
        $bmp.Dispose()
    } else {
        $bmp.Save($shotPath, [System.Drawing.Imaging.ImageFormat]::Png)
        $bmp.Dispose()
        Write-Host ("已保存全屏截图(含任务栏): {0}  {1}x{2}  ({3} KB)" -f $shotPath, $screen.Width, $screen.Height, [math]::Round((Get-Item $shotPath).Length/1KB, 1))
        $captured = $true
    }
} catch {
    Write-Host "全屏截图失败: $_"
}

# ---- 回退：PrintWindow 抓窗口矩形（至少留一张诊断图）----
if (-not $captured) {
    Write-Host "回退到 PrintWindow（窗口矩形）"
    $bmp2 = New-Object System.Drawing.Bitmap($w, $h, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g2 = [System.Drawing.Graphics]::FromImage($bmp2)
    $hdc = $g2.GetHdc()
    $ok = [MdW]::PrintWindow($hwnd, $hdc, 2)   # 2 = PW_RENDERFULLCONTENT
    $g2.ReleaseHdc($hdc)
    $g2.Dispose()
    if ($ok) {
        $bmp2.Save($shotPath, [System.Drawing.Imaging.ImageFormat]::Png)
        Write-Host ("已保存窗口截图(PrintWindow 回退): {0}  {1}x{2}  ({3} KB)" -f $shotPath, $w, $h, [math]::Round((Get-Item $shotPath).Length/1KB, 1))
    }
    $bmp2.Dispose()
}

Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue