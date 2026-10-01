#include "pch.h"
#include "core/Renderer.h"
#include "core/Theme.h"
#include "controls/NavigationView.h"
#include "utils/Easing.h"

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
        r.DrawLine(L, T + h * 0.4f, cx, T, 1.4f * s, c);
        r.DrawLine(cx, T, L + w, T + h * 0.4f, 1.4f * s, c);
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
            for (int j = 0; j < 2; ++j)
                r.StrokeRect(RectF(L + i * (cell + gap), T + j * (cell + gap), cell, cell), w, c);
        break;
    }
    case 2: { // profile（头 + 肩）
        float hr = 3.2f * s;
        r.StrokeEllipse(RectF(cx - hr, y + 2.0f * s - hr, hr * 2, hr * 2), 1.4f * s, c);
        float sw = 11.0f * s, st = y + 9.0f * s, sb = y + 15.0f * s;
        float sl = cx - sw * 0.5f, sr = cx + sw * 0.5f;
        r.DrawLine(sl, sb, sl, sb - 2.0f * s, 1.4f * s, c);
        r.DrawLine(sl, sb - 2.0f * s, cx, st, 1.4f * s, c);
        r.DrawLine(cx, st, sr, sb - 2.0f * s, 1.4f * s, c);
        r.DrawLine(sr, sb - 2.0f * s, sr, sb, 1.4f * s, c);
        break;
    }
    default: { // gear（外圆 + 中心点 + 齿）
        float orad = 5.0f * s;
        r.StrokeEllipse(RectF(cx - orad, cy - orad, orad * 2, orad * 2), 1.4f * s, c);
        float crad = 1.8f * s;
        r.FillEllipse(RectF(cx - crad, cy - crad, crad * 2, crad * 2), c);
        for (int k = 0; k < 8; ++k) {
            float a = kPi * k / 4.0f;
            float c1 = std::cos(a), s1 = std::sin(a);
            r.DrawLine(cx + c1 * orad, cy + s1 * orad,
                       cx + c1 * (orad + 2.0f * s), cy + s1 * (orad + 2.0f * s), 1.6f * s, c);
        }
        break;
    }
    }
}

// 三横线 hamburger（16x16）
void DrawHamburger(Renderer& r, float x, float y, float s, const Color& c) {
    float w = 16.0f * s, cy = y + 8.0f * s, gap = 5.0f * s;
    for (int i = -1; i <= 1; ++i)
        r.DrawLine(x, cy + i * gap, x + w, cy + i * gap, 1.4f * s, c);
}

// 分组 chevron（规格：40x36 盒内 8px 字号，展开时 rotate 180）
void DrawChevron(Renderer& r, float cx, float cy, float s, bool up, const Color& c) {
    float w = 4.0f * s, h = 2.4f * s;
    if (up) {
        r.DrawLine(cx - w, cy + h * 0.5f, cx, cy - h * 0.5f, 1.2f * s, c);
        r.DrawLine(cx, cy - h * 0.5f, cx + w, cy + h * 0.5f, 1.2f * s, c);
    } else {
        r.DrawLine(cx - w, cy - h * 0.5f, cx, cy + h * 0.5f, 1.2f * s, c);
        r.DrawLine(cx, cy + h * 0.5f, cx + w, cy - h * 0.5f, 1.2f * s, c);
    }
}

} // namespace

// ============================================================
// 内容管理
// ============================================================
void NavigationView::SyncGroups() {
    if (groupT_.size() != items_.size()) {
        groupT_.assign(items_.size(), 1.0f);
        for (size_t i = 0; i < items_.size(); ++i)
            groupT_[i] = items_[i].expanded ? 1.0f : 0.0f;
    }
}

int NavigationView::AddItem(const Item& it) {
    items_.push_back(it);
    SyncGroups();
    return static_cast<int>(items_.size()) - 1;
}

int NavigationView::AddHeader(const std::wstring& text) {
    Item it;
    it.label = text;
    it.kind = ItemKind::Header;
    return AddItem(it);
}

int NavigationView::AddSeparator() {
    Item it;
    it.kind = ItemKind::Separator;
    return AddItem(it);
}

int NavigationView::AddGroup(const std::wstring& label, int icon,
                             const std::vector<Item>& children, bool expanded) {
    Item p;
    p.label = label;
    p.icon = icon;
    p.expandable = true;
    p.expanded = expanded;
    p.selectsOnInvoked = false;   // WinUI：分组头默认只负责展开/收起
    int pi = AddItem(p);
    for (const Item& c : children) {
        Item ch = c;
        ch.isChild = true;
        ch.group = pi;
        AddItem(ch);
    }
    return pi;
}

void NavigationView::SetSettings(const std::wstring& label) {
    Item it;
    it.label = label;
    it.icon = 3;
    it.kind = ItemKind::Settings;
    AddItem(it);
}

void NavigationView::ClearItems() {
    items_.clear();
    groupT_.clear();
    placed_.clear();
    selected_ = 0;
    indX_ = -1.0f;
}

void NavigationView::SetSelectedIndex(int i) {
    if (i >= 0 && i < static_cast<int>(items_.size())) selected_ = i;
}

int NavigationView::GroupParentOf(int i) const {
    if (i < 0 || i >= static_cast<int>(items_.size())) return -1;
    if (!items_[i].isChild) return -1;
    int g = items_[i].group;
    if (g < 0 || g >= static_cast<int>(items_.size())) return -1;
    return g;
}

float NavigationView::GroupProgress(int groupIndex) const {
    if (groupIndex < 0 || groupIndex >= static_cast<int>(groupT_.size())) return 1.0f;
    return Clamp01(groupT_[groupIndex]);
}

bool NavigationView::IsGroupExpanded(int groupIndex) const {
    if (groupIndex < 0 || groupIndex >= static_cast<int>(items_.size())) return false;
    return items_[groupIndex].expanded;
}

void NavigationView::SetGroupExpanded(int groupIndex, bool expanded) {
    if (groupIndex < 0 || groupIndex >= static_cast<int>(items_.size())) return;
    if (!items_[groupIndex].expandable) return;
    items_[groupIndex].expanded = expanded;
}

// ============================================================
// 显示模式 / 面板状态
// ============================================================
void NavigationView::SetDisplayMode(DisplayMode m) {
    if (mode_ == m) return;
    mode_ = m;
    if (m == DisplayMode::Top) {
        paneT_ = 1.0f;
        paneAnim_ = false;
        paneOpen_ = true;
        compact_ = false;
        return;
    }
    compact_ = (m == DisplayMode::LeftCompact);
    paneFrom_ = paneT_;
    // 只有 Left 模式默认展开；LeftCompact/LeftMinimal 都从「收起」开始
    paneOpen_ = (m == DisplayMode::Left);
    StartPaneAnim(paneOpen_);
}

void NavigationView::SetCompact(bool c) {
    if (compact_ == c) return;
    compact_ = c;
    StartPaneAnim(!c);
}

void NavigationView::TogglePane() { StartPaneAnim(paneT_ < 0.5f); }

void NavigationView::StartPaneAnim(bool opening) {
    if (paneAnim_ && opening == paneOpen_) return;          // 同方向动画中
    if (!paneAnim_ && paneT_ == (opening ? 1.0f : 0.0f)) return; // 已到位
    paneOpen_ = opening;
    paneFrom_ = paneT_;
    paneElapsed_ = 0.0f;
    if (mode_ == DisplayMode::Left) {
        paneDur_ = 0.20f;                     // Inline：200 / 200
    } else {
        paneDur_ = opening ? 0.35f : 0.12f;   // Overlay：350 / 120
    }
    paneAnim_ = true;
}

float NavigationView::PaneWidthDip() const {
    if (mode_ == DisplayMode::Top) return 0.0f;
    float rail = (mode_ == DisplayMode::LeftMinimal) ? 0.0f : kCompactW;
    return LerpF(rail, kOpenW, Clamp01(paneT_));
}

float NavigationView::RailWidthDip() const {
    switch (mode_) {
    case DisplayMode::Top:        return 0.0f;
    case DisplayMode::LeftMinimal: return 0.0f;
    case DisplayMode::LeftCompact: return kCompactW;   // 常驻图标栏
    default:                       return PaneWidthDip(); // Left：跟随动画
    }
}

RectF NavigationView::ContentRect() const {
    float s = sc_;
    if (mode_ == DisplayMode::Top) {
        float h = kTopBarH * s;
        return RectF(bounds_.x, bounds_.y + h, bounds_.w, FzMx(0.0f, bounds_.h - h));
    }
    float rail = RailWidthDip() * s;
    return RectF(bounds_.x + rail, bounds_.y,
                 FzMx(0.0f, bounds_.w - rail), bounds_.h);
}

RectF NavigationView::TopBarRect() const {
    float s = sc_;
    return RectF(bounds_.x + kPanePad * s, bounds_.y,
                 FzMx(0.0f, bounds_.w - 2.0f * kPanePad * s), kTopBarH * s);
}

RectF NavigationView::HamburgerRect() const {
    float s = sc_;
    if (mode_ == DisplayMode::Top) return RectF();
    float pad = kPanePad * s;
    float innerW = FzMx(0.0f, PaneWidthDip() * s - 2.0f * pad);
    // 规格：.has-pane-title 时按钮宽度 = OpenPaneLength - 8 = 312；无标题/紧凑时 40
    float btnW = FzMx(innerW, kHamW * s);
    float x = bounds_.x + pad;
    float y = bounds_.y + pad + kItemM * s;
    if (mode_ == DisplayMode::LeftMinimal) {
        // Minimal：不占位，汉堡浮在左上角（面板收起时也要可见）
        x = bounds_.x + pad;
        y = bounds_.y + pad;
    }
    return RectF(x, y, btnW, kHamH * s);
}

RectF NavigationView::PlacedRect(int i) const {
    if (i < 0 || i >= static_cast<int>(placed_.size())) return RectF();
    return placed_[i].rect;
}

float NavigationView::IndicatorTargetX() const {
    float rel = kIndLeft;
    if (selected_ >= 0 && selected_ < static_cast<int>(items_.size()) && items_[selected_].isChild)
        rel = kIndChildL;   // 规格：.is-child { left: 36px }
    return bounds_.x + rel * sc_;
}

// ============================================================
// 布局
// ============================================================
void NavigationView::RebuildLayout() {
    SyncGroups();
    placed_.assign(items_.size(), Placed());
    if (items_.empty() || bounds_.w <= 0.0f || bounds_.h <= 0.0f) return;

    float s = sc_;

    // ---------- 顶部导航 ----------
    if (mode_ == DisplayMode::Top) {
        RectF bar = TopBarRect();
        float itemW = kTopBarH * s;         // 48x48 图标项
        float x = bar.x + kPanePad * s;
        float rightX = bar.Right() - kPanePad * s;
        for (size_t i = 0; i < items_.size(); ++i) {
            const Item& it = items_[i];
            if (it.kind != ItemKind::Item) continue;   // 顶栏不显示 header/separator/settings
            if (it.isChild) continue;
            if (!it.enabled) continue;
            placed_[i].rect = RectF(x, bar.y, itemW, bar.h);
            x += itemW;
        }
        for (size_t i = 0; i < items_.size(); ++i) {
            if (items_[i].kind != ItemKind::Settings) continue;
            rightX -= itemW;
            placed_[i].rect = RectF(rightX, bar.y, itemW, bar.h);
        }
        return;
    }

    // ---------- 左侧面板 ----------
    float pad = kPanePad * s;
    float px = bounds_.x;
    float innerW = FzMx(0.0f, PaneWidthDip() * s - 2.0f * pad);
    float y = bounds_.y + pad + kHamRowH * s;   // command row

    for (size_t i = 0; i < items_.size(); ++i) {
        Item& it = items_[i];
        if (it.kind == ItemKind::Settings) continue;   // 固定底部，稍后处理

        if (it.kind == ItemKind::Header) {
            if (paneT_ < 0.02f) continue;              // 紧凑态：组标题高度 0
            placed_[i].rect = RectF(px + pad, y, innerW, kHeaderH * s);
            placed_[i].alpha = paneT_;
            y += kHeaderH * s;
            continue;
        }
        if (it.kind == ItemKind::Separator) {
            placed_[i].rect = RectF(px + pad, y + kSepMT * s, innerW, kSepH * s);
            placed_[i].alpha = paneT_;
            y += (kSepMT + kSepH + kSepMB) * s;
            continue;
        }

        // 普通项 / 分组子项
        float gp = 1.0f;
        int parent = GroupParentOf(static_cast<int>(i));
        if (parent >= 0) {
            // 规格 isPaneGroupChildrenVisible = !isClosedCompact：
            // 面板处于紧凑（收起）态时，分组子项整组高度归 0（随面板一起收起）
            gp = GroupProgress(parent) * Clamp01(paneT_);
            if (gp <= 0.001f) continue;
        }

        float h = kItemH * s * gp;
        placed_[i].rect = RectF(px + pad, y + kItemM * s * gp, innerW, h);
        placed_[i].alpha = gp;
        y += (kItemH + 2.0f * kItemM) * s * gp;        // 行距 40
    }

    // Settings：固定底部
    for (size_t i = 0; i < items_.size(); ++i) {
        if (items_[i].kind != ItemKind::Settings) continue;
        placed_[i].rect = RectF(px + pad,
                                bounds_.y + bounds_.h - pad - kItemM * s - kItemH * s,
                                innerW, kItemH * s);
    }
}

// ============================================================
// 输入
// ============================================================
int NavigationView::HitItem(float x, float y) const {
    for (size_t i = 0; i < placed_.size(); ++i) {
        if (placed_[i].rect.IsEmpty()) continue;
        ItemKind k = items_[i].kind;
        if (k == ItemKind::Header || k == ItemKind::Separator) continue;
        if (placed_[i].rect.Contains(x, y)) return static_cast<int>(i);
    }
    return -1;
}

void NavigationView::ActivateItem(int i, float x) {
    if (i < 0 || i >= static_cast<int>(items_.size())) return;
    const Item& it = items_[i];
    if (!it.enabled) return;

    // 1) 点 chevron：只切换展开
    if (it.expandable && static_cast<size_t>(i) < placed_.size()) {
        RectF r = placed_[i].rect;
        if (x >= r.Right() - kChevW * sc_) {
            SetGroupExpanded(i, !it.expanded);
            return;
        }
    }
    // 2) 点主体
    if (it.selectsOnInvoked) {
        selected_ = i;
        if (it.expandable && !it.expanded) SetGroupExpanded(i, true); // 选中时确保展开
        if (onSelection_) onSelection_(i);
    } else if (it.expandable) {
        SetGroupExpanded(i, !it.expanded);
    }
}

void NavigationView::OnMouseMove(float x, float y) {
    hot_ = false;
    hotIndex_ = -1;
    hamburgerHot_ = false;
    RebuildLayout();
    RectF hr = HamburgerRect();
    if (!hr.IsEmpty() && hr.Contains(x, y)) {
        hot_ = true;
        hamburgerHot_ = true;
        return;
    }
    int i = HitItem(x, y);
    if (i >= 0) {
        hot_ = true;
        hotIndex_ = i;
    }
}

void NavigationView::OnMouseLeave() {
    hot_ = false;
    hotIndex_ = -1;
    hamburgerHot_ = false;
}

void NavigationView::OnMouseDown(float x, float y) {
    RebuildLayout();
    RectF hr = HamburgerRect();
    hamburgerPressed_ = (!hr.IsEmpty() && hr.Contains(x, y));
    if (hamburgerPressed_) {
        hot_ = true;
        hamburgerHot_ = true;
        return;
    }
    pressIndex_ = HitItem(x, y);
    if (pressIndex_ >= 0) {
        hot_ = true;
        hotIndex_ = pressIndex_;
    }
}

void NavigationView::OnMouseUp(float x, float y) {
    RebuildLayout();
    RectF hr = HamburgerRect();
    if (hamburgerPressed_) {
        hamburgerPressed_ = false;
        if (!hr.IsEmpty() && hr.Contains(x, y)) {
            TogglePane();
            return;
        }
    }
    int prev = pressIndex_;
    pressIndex_ = -1;
    if (prev < 0) return;
    int i = HitItem(x, y);
    if (i >= 0 && i == prev) ActivateItem(i, x);
}

// ============================================================
// 动画
// ============================================================
bool NavigationView::Update(float dt) {
    SyncGroups();
    bool busy = false;

    // ---- 面板宽/可见度（WinUI 契约：Inline 200/200，Overlay 350/120）----
    if (paneAnim_) {
        paneElapsed_ += dt;
        float p = (paneDur_ > 0.0f) ? FzMn(1.0f, paneElapsed_ / paneDur_) : 1.0f;
        float e = (mode_ == DisplayMode::Left) ? EaseNavInline(p) : EaseNavOverlay(p);
        paneT_ = LerpF(paneFrom_, paneOpen_ ? 1.0f : 0.0f, e);
        if (p >= 1.0f) {
            paneT_ = paneOpen_ ? 1.0f : 0.0f;
            paneAnim_ = false;
        }
        busy = true;
    } else {
        paneT_ = paneOpen_ ? 1.0f : 0.0f;
    }

    // ---- 分组展开（height transition ≈ 200ms）----
    for (size_t i = 0; i < items_.size(); ++i) {
        if (!items_[i].expandable) continue;
        float t = items_[i].expanded ? 1.0f : 0.0f;
        float v = Approach(groupT_[i], t, dt, 14.0f);
        if (std::abs(v - t) < 0.002f) v = t;
        if (std::abs(v - groupT_[i]) > 0.0002f) busy = true;
        groupT_[i] = v;
    }

    // ---- hover / press 过渡（--fast-duration）----
    float ht = hot_ ? 1.0f : 0.0f;
    float hb = hamburgerHot_ ? 1.0f : 0.0f;
    float v1 = Approach(hotT_, ht, dt, 30.0f);
    float v2 = Approach(hamburgerT_, hb, dt, 30.0f);
    if (std::abs(v1 - ht) < 0.002f) v1 = ht;
    if (std::abs(v2 - hb) < 0.002f) v2 = hb;
    if (std::abs(v1 - hotT_) > 0.0002f || std::abs(v2 - hamburgerT_) > 0.0002f) busy = true;
    hotT_ = v1;
    hamburgerT_ = v2;

    // ---- indicator left（transition 200ms）----
    float tx = IndicatorTargetX();
    if (indX_ < 0.0f) {
        indX_ = tx;
    } else {
        float v = Approach(indX_, tx, dt, 14.0f);
        if (std::abs(v - tx) < 0.1f) v = tx;
        if (std::abs(v - indX_) > 0.01f) busy = true;
        indX_ = v;
    }

    return busy;
}

// ============================================================
// 绘制
// ============================================================
void NavigationView::Draw(Renderer& renderer, const Theme& theme, float scale) {
    float s = scale;
    sc_ = s;
    RebuildLayout();

    const bool light = theme.lightMode;
    const Color subtleHov   = light ? Color(0, 0, 0, 0.0373f) : Color(1, 1, 1, 0.0605f);
    const Color subtlePress = light ? Color(0, 0, 0, 0.0241f) : Color(1, 1, 1, 0.0419f);
    const Color divider     = light ? Color(0, 0, 0, 0.06f)   : Color(1, 1, 1, 0.08f);
    const Color textPrimary   = theme.TextPrimary();
    const Color textSecondary = theme.TextSecondary();
    const Color textDisabled  = theme.TextDisabled();
    const Color accent     = theme.Accent();
    const Color navBg      = theme.WindowBg();
    const Color contentBg  = theme.NavContentBg();
    const Color stroke     = theme.CardBorder();

    const bool overlayPane = (mode_ == DisplayMode::LeftCompact || mode_ == DisplayMode::LeftMinimal);
    const float paneW = PaneWidthDip() * s;
    const RectF cr = ContentRect();

    // ---- 1) 导航整体底色 ----
    renderer.FillRect(bounds_, navBg);

    // ---- 2) 内容区：左上 8px 圆角 + 上/左 1px 描边 ----
    if (cr.w > 0.5f && cr.h > 0.5f) {
        const float rad = kContentR * s;
        renderer.FillRoundedRect(cr, rad, contentBg);
        // 压掉其余三角（内容底色不透明，无二次叠加问题）
        renderer.FillRect(RectF(cr.x + rad, cr.y, FzMx(0.0f, cr.w - rad), rad), contentBg);
        renderer.FillRect(RectF(cr.x, cr.y + rad, rad, FzMx(0.0f, cr.h - rad)), contentBg);
        renderer.FillRect(RectF(cr.x + rad, cr.y + cr.h - rad, FzMx(0.0f, cr.w - rad), rad), contentBg);
        renderer.FillRect(RectF(cr.x + cr.w - rad, cr.y + rad, rad, FzMx(0.0f, cr.h - 2.0f * rad)), contentBg);
        // 描边：上 + 左（含左上 1/4 圆弧）
        renderer.DrawLine(cr.x + rad, cr.y, cr.x + cr.w, cr.y, 1.0f * s, stroke);
        renderer.DrawLine(cr.x, cr.y + rad, cr.x, cr.y + cr.h, 1.0f * s, stroke);
        const int N = 6;
        float pcx = cr.x + rad, pcy = cr.y + rad;
        float px0 = cr.x, py0 = cr.y + rad;
        for (int i = 1; i <= N; ++i) {
            float a = kPi * (1.0f + 0.5f * static_cast<float>(i) / static_cast<float>(N));
            float qx = pcx + rad * std::cos(a);
            float qy = pcy + rad * std::sin(a);
            renderer.DrawLine(px0, py0, qx, qy, 1.0f * s, stroke);
            px0 = qx;
            py0 = qy;
        }
    }

    // ---- 3) 覆盖模式下的面板（画在内容之上）+ 右缘描边 ----
    if (overlayPane && paneW > 1.0f && paneW > RailWidthDip() * s + 1.0f) {
        renderer.FillRect(RectF(bounds_.x, bounds_.y, paneW, bounds_.h), navBg);
        renderer.DrawLine(bounds_.x + paneW, bounds_.y, bounds_.x + paneW, bounds_.y + bounds_.h,
                          1.0f * s, stroke);
    }

    // ---- 4) 菜单项 ----
    for (size_t i = 0; i < items_.size(); ++i) {
        const Item& it = items_[i];
        const RectF r = placed_[i].rect;
        if (r.IsEmpty()) continue;
        const float a = placed_[i].alpha;
        if (a <= 0.01f) continue;

        // 4a) 组标题
        if (it.kind == ItemKind::Header) {
            renderer.DrawText(it.label, r.x + kHeaderPadX * s, r.y,
                              FzMx(0.0f, r.w - 2.0f * kHeaderPadX * s), r.h,
                              L"Segoe UI", 14.0f * s, DWRITE_FONT_WEIGHT_SEMI_BOLD,
                              textSecondary.WithAlpha(a),
                              DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
            continue;
        }
        // 4b) 分隔线
        if (it.kind == ItemKind::Separator) {
            renderer.FillRect(RectF(r.x, r.y, r.w, r.h), divider.WithAlpha(a));
            continue;
        }

        // 4c) 菜单项
        const bool sel = (static_cast<int>(i) == selected_);
        const bool dis = !it.enabled;
        const bool hot = (hotIndex_ == static_cast<int>(i));
        const bool prs = (pressIndex_ == static_cast<int>(i));

        Color bg(0, 0, 0, 0.0f);
        if (!dis) {
            if (mode_ == DisplayMode::Top) {
                // 规格：Top 模式选中背景为透明，选中只靠 indicator
                if (prs) bg = subtlePress;
                else if (hot) bg = subtleHov;
            } else if (prs) {
                bg = sel ? subtleHov : subtlePress;
            } else if (hot) {
                bg = sel ? subtlePress : subtleHov;
            } else if (sel) {
                bg = subtleHov;
            }
        }
        if (bg.a > 0.001f)
            renderer.FillRoundedRect(r, kRadius * s, bg.WithAlpha(bg.a * a));

        // 图标：展开态 padding-left 12（子项 44）；紧凑态宽度收窄后自然居中
        const float padL = (it.isChild ? kChildPadL : kItemPadX) * s;
        const float iconY = r.y + (r.h - kIconSize * s) * 0.5f;
        const float iconX = (mode_ == DisplayMode::Top)
                                ? r.CenterX() - kIconSize * s * 0.5f
                                : r.x + padL;
        const Color fg = dis ? textDisabled : textPrimary;
        DrawIcon(renderer, it.icon, iconX, iconY, s, fg.WithAlpha(fg.a * a));

        // 标签（紧凑/收起时随 paneT_ 淡出）
        float labelAlpha = (mode_ == DisplayMode::Top) ? 0.0f : (paneT_ * a);
        if (labelAlpha > 0.03f && !it.label.empty()) {
            const float lx = iconX + (kIconSize + kIconGap) * s;
            const float maxW = FzMx(0.0f, r.Right() - kItemPadX * s - lx);
            renderer.DrawText(it.label, lx, r.y, maxW, r.h,
                              L"Segoe UI", 14.0f * s, DWRITE_FONT_WEIGHT_NORMAL,
                              fg.WithAlpha(fg.a * labelAlpha),
                              DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        }

        // 分组 chevron（紧凑态隐藏）
        if (it.expandable && !it.isChild && paneT_ > 0.3f) {
            bool up = GroupProgress(static_cast<int>(i)) > 0.5f;
            DrawChevron(renderer, r.Right() - kChevRH * s, r.CenterY(), s, up,
                        fg.WithAlpha(fg.a * a * paneT_));
        }
    }

    // ---- 5) indicator ----
    if (selected_ >= 0 && selected_ < static_cast<int>(items_.size())) {
        const RectF sr = placed_[selected_].rect;
        if (!sr.IsEmpty() && items_[selected_].kind == ItemKind::Item) {
            if (mode_ == DisplayMode::Top) {
                // 规格：Top 模式 bottom 4px、高 3px、长 16px（居中于选中项）
                const float w = kIndLen * s;
                renderer.FillRoundedRect(RectF(sr.CenterX() - w * 0.5f,
                                               sr.Bottom() - kPanePad * s - kIndThick * s,
                                               w, kIndThick * s),
                                         1.5f * s, accent);
            } else {
                float ix = (indX_ >= 0.0f) ? indX_ : IndicatorTargetX();
                renderer.FillRoundedRect(RectF(ix, sr.CenterY() - kIndLen * s * 0.5f,
                                               kIndThick * s, kIndLen * s),
                                         2.0f * s, accent);
            }
        }
    }

    // ---- 6) command row：hamburger + PaneTitle ----
    if (mode_ != DisplayMode::Top) {
        const RectF hr = HamburgerRect();
        if (!hr.IsEmpty()) {
            // 只有 hover/press 背景是条件绘制；图标必须常显
            const Color hb = hamburgerPressed_ ? subtlePress : subtleHov;
            const float alpha = FzMx(hamburgerT_, mode_ == DisplayMode::LeftMinimal ? paneT_ : 0.0f);
            if (alpha > 0.01f)
                renderer.FillRoundedRect(hr, kRadius * s, hb.WithAlpha(hb.a * alpha));

            // 规格：图标 margin 0 12px（紧凑宽度 40 时恰好水平居中）
            const float hx = hr.x + kItemPadX * s;
            const float hy = hr.y + (hr.h - 16.0f * s) * 0.5f;
            DrawHamburger(renderer, hx, hy, s, textPrimary);

            const float la = paneT_ * paneT_;
            if (la > 0.03f && !paneTitle_.empty()) {
                const float lx = hx + (16.0f + 12.0f) * s;   // icon + margin 12
                renderer.DrawText(paneTitle_, lx, hr.y, FzMx(0.0f, hr.Right() - lx), hr.h,
                                  L"Segoe UI", 14.0f * s, DWRITE_FONT_WEIGHT_SEMI_BOLD,
                                  textPrimary.WithAlpha(la),
                                  DWRITE_TEXT_ALIGNMENT_LEADING,
                                  DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
            }
        }
    }
}

} // namespace ModernDesign