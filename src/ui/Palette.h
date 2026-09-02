#pragma once
#include <QColor>

namespace autoflow {

// 设计系统：shadcn/ui（zinc 中性色系 + blue primary）
// 中性色取自 Tailwind zinc 阶（--background / --foreground / --card / --muted / --border...），
// 强调色取自 blue 阶（--primary），状态语义色保持 AA 安全；禁用玻璃/渐变/霓虹等 AI 味。
namespace Palette {

// ---- 画布/背景（zinc）----
inline QColor bg(bool dark)       { return dark ? QColor("#09090B") : QColor("#FFFFFF"); }  // --background（zinc-950 / white）
inline QColor surface(bool dark)  { return dark ? QColor("#09090B") : QColor("#FFFFFF"); }  // --card
inline QColor surface2(bool dark) { return dark ? QColor("#18181B") : QColor("#F4F4F5"); }  // zinc-900 / zinc-100（--secondary）
inline QColor surface3(bool dark) { return dark ? QColor("#27272A") : QColor("#E4E4E7"); }  // zinc-800 / zinc-200（--muted）
inline QColor border(bool dark)   { return dark ? QColor("#27272A") : QColor("#E4E4E7"); }  // --border（zinc-800 / zinc-200）
inline QColor borderStrong(bool dark) { return dark ? QColor("#3F3F46") : QColor("#D4D4D8"); }  // --input（zinc-700 / zinc-300）

// ---- 文字（zinc）----
inline QColor text(bool dark)         { return dark ? QColor("#FAFAFA") : QColor("#09090B"); }  // --foreground（zinc-50 / zinc-950）
inline QColor textPrimary(bool dark)  { return text(dark); }
inline QColor textSecondary(bool dark){ return dark ? QColor("#A1A1AA") : QColor("#71717A"); }  // --muted-foreground（zinc-400 / zinc-500）
inline QColor textTertiary(bool dark) { return dark ? QColor("#71717A") : QColor("#A1A1AA"); }  // zinc-500 / zinc-400
inline QColor textDisabled(bool dark) { return dark ? QColor("#52525B") : QColor("#D4D4D8"); }  // zinc-600 / zinc-300
// 旧名别名（避免全工程大规模替换）
inline QColor textDim(bool dark)  { return textSecondary(dark); }
inline QColor textMute(bool dark) { return textTertiary(dark); }

// ---- 强调色（blue primary · 选中/聚焦/主按钮）----
inline QColor accent(bool dark)        { return dark ? QColor("#3B82F6") : QColor("#2563EB"); }  // --primary（blue-500 / blue-600）
inline QColor accentHover(bool dark)   { return dark ? QColor("#60A5FA") : QColor("#1D4ED8"); }  // blue-400 / blue-700
inline QColor accentPressed(bool dark) { return dark ? QColor("#2563EB") : QColor("#1E40AF"); }  // blue-600 / blue-800
inline QColor accentSubtle(bool dark)  { return dark ? QColor(59,130,246,40) : QColor(37,99,235,30); }  // 选中/聚焦浅底
inline QColor accentBorder(bool dark)  { return dark ? QColor(96,165,250,90) : QColor("#BFDBFE"); }  // --ring 弱化（blue-400 / blue-200）

// ---- 状态语义色（AA 安全）----
inline QColor success(bool dark) { return dark ? QColor("#4ADE80") : QColor("#16A34A"); }  // 成功
inline QColor warning(bool dark) { return dark ? QColor("#FBBF24") : QColor("#B45309"); }  // 警告
inline QColor danger(bool dark)  { return dark ? QColor("#F87171") : QColor("#DC2626"); }   // 失败
inline QColor info(bool dark)    { return dark ? QColor("#22D3EE") : QColor("#0891B2"); }   // 信息
// 浅底（badge/日志行底色）
inline QColor successSubtle(bool dark) { return dark ? QColor(74,222,128,36)  : QColor("#DCFCE7"); }
inline QColor warningSubtle(bool dark) { return dark ? QColor(251,191,36,36)  : QColor("#FEF3C7"); }
inline QColor dangerSubtle(bool dark)  { return dark ? QColor(248,113,113,36) : QColor("#FEE2E2"); }
inline QColor infoSubtle(bool dark)    { return dark ? QColor(34,211,238,32)  : QColor("#CFFAFE"); }
// 旧名别名
inline QColor run(bool dark)  { return success(dark); }
inline QColor warn(bool dark) { return warning(dark); }
inline QColor stop(bool dark) { return danger(dark); }

// ---- 遮罩 ----
inline QColor scrim(bool dark) { return dark ? QColor(0,0,0,140) : QColor(0,0,0,80); }

// ---- 节点 ----
inline QColor nodeBg(bool dark)      { return surface(dark); }
inline QColor nodeBorder(bool dark)  { return borderStrong(dark); }
inline QColor nodeTitle(bool dark)   { return text(dark); }
inline QColor nodeText(bool dark)    { return textDim(dark); }
inline QColor nodeDivider(bool dark) { return border(dark); }
inline QColor nodeShadow(bool dark)  { return dark ? QColor(0, 0, 0, 120) : QColor(0, 0, 0, 18); }
inline QColor nodeSelectedBg(bool dark)     { return accentSubtle(dark); }
inline QColor nodeSelectedBorder(bool dark) { return accent(dark); }

// ---- 端口 ----
inline QColor portFill(bool dark) { return surface(dark); }
inline QColor portRing(bool dark) { return textDim(dark); }

// ---- 连线 ----
inline QColor edge(bool dark) { return borderStrong(dark); }

// ---- 画布 ----
inline QColor canvasBg(bool dark)        { return bg(dark); }
inline QColor canvasGridMinor(bool dark) { return dark ? QColor(255, 255, 255, 10) : QColor(0, 0, 0, 8); }
inline QColor canvasGridMajor(bool dark) { return dark ? QColor(255, 255, 255, 22) : QColor(0, 0, 0, 14); }

} // namespace Palette
} // namespace autoflow
