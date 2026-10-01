#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/NavigationView.h"

namespace ModernDesign {

namespace {

// 在 16x16 盒 (x,y) 内绘制矢量图标（无 Segoe Fluent Icons 字体保证，故手绘）
void DrawIcon(Renderer& r, int kind, float x, float y, float s, const Color& c) {
    float sz = 16.0f * s;
    float cx = x + sz * 0.5f, cy = y + sz * 0.5f;
    switch (kind) {
    case 0: { // home
        float w = 13.0f * s, h = 12.0f * s;
        float L = x + (sz - w) * 0.5f, T = y + (sz - h) * 0.5f;
        // 屋顶（三角）
        r.DrawLine(L, T + h * 0.4f, cx, T, 1.4f * s, c);
        r.DrawLine(cx, T, L + w, T + h * 0.4f, 1.4f * s, c);
        // 房身
        float bw = 9.0f * s, bt = T + h * 0.35f, bh = h * 0.65f;
        float bl = cx - bw * 0.5f;
        r.DrawLine(bl, bt, bl, bt + bh, 1.4f * s, c);
        r.DrawLine(bl + bw, bt, bl + bw, bt + bh, 1.4f * s, c);
        r.DrawLine(bl, bt + bh, bl + bw, bt + bh, 1.4f * s, c);
        break;
    }
    case 1: { // grid（4 方块）
        float g = 13.0f * s, cell = g * 0.42f, gap = g * 0.16f;
        float L = x + (sz - g) * 0.5f, T = y + (sz - g) * 0.5f;
        float w = 1.3f * s;
        for (int i = 0; i < 2; ++i)
            for (int j = 0; j < 2; ++j) {
                float bx = L + i * (cell + gap), by = T + j * (cell + gap);
                r.StrokeRect(RectF(bx, by, cell, cell), w, c);
            }
        break;
    }
    case 2: { // profile（头 + 肩）
        float hr = 3.2f * s;
        r.StrokeEllipse(RectF(cx - hr, y + 2.0f * s - hr, hr * 2, hr * 2), 1.4f * s, c);
        float sw = 11.0f * s, st = y + 9.0f * s, sb = y + 15.0f * s;
        float sl = cx - sw * 0.5f, sr = cx + sw * 0.5f;
        // 肩：用椭圆弧近似（三段线）
        r.DrawLine(sl, sb, sl, sb - 2.0f * s, 1.4f * s, c);
        r.DrawLine(sl, sb - 2.0f * s, cx, st, 1.4f * s, c);
        r.DrawLine(cx, st, sr, sb - 2.0f * s, 1.4f * s, c);
        r.DrawLine(sr, sb - 2.0f * s, sr, sb, 1.4f * s, c);
        break;
    }
    default: { // gear（简化：外圆 + 中心点）
        float orad = 5.0f * s;
        r.StrokeEllipse(RectF(cx - orad, cy - orad, orad * 2, orad * 2), 1.4f * s, c);
        float crad = 1.8f * s;
        r.FillEllipse(RectF(cx - crad, cy - crad, crad * 2, crad * 2), c);
        // 8 个齿（短放射线）
        for (int k = 0; k < 8; ++k) {
            float a = kPi * k / 4.0f;
            float c1 = std::cos(a), s1 = std::sin(a);
            r.DrawLine(cx + c1 * orad, cy + s1 * orad, cx + c1 * (orad + 2.0f * s), cy + s1 * (orad + 2.0f * s), 1.6f * s, c);
        }
        break;
    }
    }
}

// 三横线 hamburger
void DrawHamburger(Renderer& r, float x, float y, float s, const Color& c) {
    float w = 16.0f * s;
    float L = x, cy = y + 8.0f * s;
    float gap = 5.0f * s;
    for (int i = -1; i <= 1; ++i)
        r.DrawLine(L, cy + i * gap, L + w, cy + i * gap, 1.4f * s, c);
}

} // namespace

float NavigationView::PaneWidthDip() const {
    float e = EaseOut(paneT_);
    return kCompactW + (kOpenW - kCompactW) * e;
}

RectF NavigationView::HamburgerRect() const {
    float s = sc_;
    float pad = kPanePad * s;
    float w = PaneWidthDip() * s - 2.0f * pad;
    return RectF(bounds_.x + pad, bounds_.y + pad + kItemM * s, w, kHamH * s);
}

RectF NavigationView::ItemRect(int i) const {
    float s = sc_;
    float pad = kPanePad * s;
    float w = PaneWidthDip() * s - 2.0f * pad;
    float y;
    if (items_[i].isSettings) {
        y = bounds_.y + bounds_.h - pad - kItemM * s - kItemH * s;
    } else {
        y = bounds_.y + pad + kHamRowH * s + i * kItemPitch * s + kItemM * s;
    }
    return RectF(bounds_.x + pad, y, w, kItemH * s);
}

void NavigationView::OnMouseMove(float x, float y) {
    hot_ = false; hotIndex_ = -1; hamburgerHot_ = false;
    if (HamburgerRect().Contains(x, y)) { hot_ = true; hamburgerHot_ = true; return; }
    for (int i = 0; i < (int)items_.size(); ++i) {
        if (ItemRect(i).Contains(x, y)) { hot_ = true; hotIndex_ = i; break; }
    }
}

void NavigationView::OnMouseLeave() { hot_ = false; hotIndex_ = -1; hamburgerHot_ = false; }

void NavigationView::OnMouseDown(float x, float y) {
    if (HamburgerRect().Contains(x, y)) { hot_ = true; hamburgerHot_ = true; return; }
    for (int i = 0; i < (int)items_.size(); ++i)
        if (ItemRect(i).Contains(x, y)) { hot_ = true; hotIndex_ = i; break; }
}

void NavigationView::OnMouseUp(float x, float y) {
    if (HamburgerRect().Contains(x, y)) { TogglePane(); }
    else for (int i = 0; i < (int)items_.size(); ++i)
        if (ItemRect(i).Contains(x, y)) {
            selected_ = i;
            if (onSelection_) onSelection_(i);
            break;
        }
}

bool NavigationView::Update(float dt) {
    float target = compact_ ? 0.0f : 1.0f;
    paneT_ = Approach(paneT_, target, dt, 20.0f);
    hotT_ = Approach(hotT_, hot_ ? 1.0f : 0.0f, dt, 30.0f);
    hamburgerT_ = Approach(hamburgerT_, hamburgerHot_ ? 1.0f : 0.0f, dt, 30.0f);
    return std::abs(paneT_ - target) > 0.005f
        || std::abs(hotT_ - (hot_ ? 1.0f : 0.0f)) > 0.005f
        || std::abs(hamburgerT_ - (hamburgerHot_ ? 1.0f : 0.0f)) > 0.005f;
}

void NavigationView::Draw(Renderer& renderer, const Theme& theme, float scale) {
    float s = scale;
    bool light = theme.lightMode;

    // ---- WinUIonWeb 精确规格 (NavigationView.vue + theme.css) ----
    // subtle-secondary 浅色(0,0,0,0.0373) 深色(255,255,255,0.0605)
    // subtle-tertiary  浅色(0,0,0,0.0241) 深色(255,255,255,0.0419)
    // stroke-divider   浅色(0,0,0,0.06)   深色(255,255,255,0.08)
    Color subtleHov   = light ? Color(0, 0, 0, 0.0373f)   : Color(1, 1, 1, 0.0605f);
    Color subtlePress = light ? Color(0, 0, 0, 0.0241f)   : Color(1, 1, 1, 0.0419f);
    Color divider     = light ? Color(0, 0, 0, 0.06f)     : Color(1, 1, 1, 0.08f);
    Color textPrimary = theme.TextPrimary();
    Color textSecondary = theme.TextSecondary();
    Color accent = theme.Accent();

    float e = EaseOut(paneT_);
    float paneW = PaneWidthDip() * s;
    float pad = kPanePad * s;
    float px = bounds_.x, py = bounds_.y;

    // 内容区背景 + 面板背景
    renderer.FillRect(RectF(px + paneW, py, FzMx(0.0f, bounds_.x + bounds_.w - px - paneW), bounds_.h), theme.ContentBg());
    renderer.FillRect(RectF(px, py, paneW, bounds_.h), theme.WindowBg());
    // 右侧 1px 分隔
    renderer.DrawLine(px + paneW, py, px + paneW, py + bounds_.h, 1.0f * s, theme.CardBorder());

    auto itemBg = [&](bool pressed, bool selected) -> Color {
        if (selected && pressed) return subtleHov;
        if (pressed) return subtlePress;
        if (selected) return subtleHov;
        return Color(0, 0, 0, 0.0f);
    };

    // ---- 菜单项 ----
    for (int i = 0; i < (int)items_.size(); ++i) {
        RectF r = ItemRect(i);
        if (r.w <= 1.0f || r.h <= 1.0f) continue;
        bool sel = (i == selected_);
        bool hot = (hotIndex_ == i);
        if (sel || hot)
            renderer.FillRoundedRect(r, 4.0f * s, itemBg(hot, sel));

        // 图标位置：展开=左(12)，紧凑=居中
        float leftIconX = r.x + 12.0f * s;
        float centeredIconX = r.x + r.w * 0.5f - 8.0f * s;
        float iconX = centeredIconX + (leftIconX - centeredIconX) * e;
        float iconY = r.y + (r.h - 16.0f * s) * 0.5f;
        Color iconCol = sel ? textPrimary : textSecondary;
        if (sel) iconCol = accent; // 选中项图标用 accent（WinUI 观感）
        DrawIcon(renderer, items_[i].icon, iconX, iconY, s, iconCol);

        // 标签（紧凑时淡出）
        if (e > 0.05f) {
            Color lc = sel ? textPrimary : textSecondary;
            lc = lc.WithAlpha(e);
            float labelX = iconX + 16.0f * s + 16.0f * s; // icon + margin-right 16
            renderer.DrawText(items_[i].label, labelX, r.y, r.x + r.w - labelX, r.h,
                              L"Segoe UI", 14.0f * s, DWRITE_FONT_WEIGHT_NORMAL, lc,
                              DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        }
    }

    // ---- accent indicator（选中项，展开时）----
    if (e > 0.5f && selected_ >= 0) {
        RectF sr = ItemRect(selected_);
        float iy = sr.y + sr.h * 0.5f - 8.0f * s;
        renderer.FillRoundedRect(RectF(px + pad, iy, 3.0f * s, 16.0f * s), 2.0f * s, accent);
    }

    // ---- hamburger 行 ----
    {
        RectF hr = HamburgerRect();
        if (hamburgerHot_)
            renderer.FillRoundedRect(hr, 4.0f * s, subtleHov.WithAlpha(hamburgerT_));
        float leftIconX = hr.x + 12.0f * s;
        float centeredIconX = hr.x + hr.w * 0.5f - 8.0f * s;
        float hx = centeredIconX + (leftIconX - centeredIconX) * e;
        float hy = hr.y + (hr.h - 16.0f * s) * 0.5f;
        DrawHamburger(renderer, hx, hy, s, textPrimary);
        if (e > 0.05f) {
            Color lc = textPrimary; lc = lc.WithAlpha(e);
            float labelX = hx + 16.0f * s + 16.0f * s;
            renderer.DrawText(paneTitle_, labelX, hr.y, hr.x + hr.w - labelX, hr.h,
                              L"Segoe UI", 14.0f * s, DWRITE_FONT_WEIGHT_SEMI_BOLD, lc,
                              DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        }
    }
}

} // namespace ModernDesign