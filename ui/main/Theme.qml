pragma Singleton
import QtQuick

QtObject {
    // 当前主题标志
    property bool dark: false

    // 亮色主题配色 (保持原有)
    readonly property color lightBg: "#FBF9F8"
    readonly property color lightTitleBg: "#F9F6F4"
    readonly property color lightSidebarBg: "#F8F5F2"
    readonly property color lightSurface: "#FFFFFF"
    readonly property color lightBorder: "#D5CDC3"
    readonly property color lightBorderInner: "#EFE9E3"
    readonly property color lightTextDark: "#2C2C2C"
    readonly property color lightTextMid: "#666666"
    readonly property color lightTextLight: "#999999"
    readonly property color lightAccent: "#C4A882"
    readonly property color lightBtnNormalBg: "#FCFAF8"
    readonly property color lightBtnNormalBorder: "#EBE4DC"
    readonly property color lightBtnPrimaryBg: "#F0E8E0"
    readonly property color lightBtnPrimaryBorder: "#DDD5CC"
    readonly property color lightBtnDangerBg: "#FDF5F5"
    readonly property color lightBtnDangerBorder: "#E8D0D0"

    // 暗色主题配色 (保持原有)
    readonly property color darkBg: "#1E1E2F"
    readonly property color darkTitleBg: "#2A2A3E"
    readonly property color darkSidebarBg: "#25253A"
    readonly property color darkSurface: "#2C2C47"
    readonly property color darkBorder: "#3D3D5C"
    readonly property color darkBorderInner: "#33334F"
    readonly property color darkTextDark: "#F1F1F6"
    readonly property color darkTextMid: "#C0C0E0"
    readonly property color darkTextLight: "#9090B0"
    readonly property color darkAccent: "#2DD4BF"
    readonly property color darkBtnNormalBg: "#2C2C47"
    readonly property color darkBtnNormalBorder: "#444466"
    readonly property color darkBtnPrimaryBg: "#2DD4BF"
    readonly property color darkBtnPrimaryBorder: "#40E0D0"
    readonly property color darkBtnDangerBg: "#4A2A2A"
    readonly property color darkBtnDangerBorder: "#6B4040"

    // ========== 原有动态主题颜色 ==========
    property color bg:                dark ? darkBg                : lightBg
    property color titleBg:           dark ? darkTitleBg           : lightTitleBg
    property color sidebarBg:         dark ? darkSidebarBg         : lightSidebarBg
    property color surface:           dark ? darkSurface           : lightSurface
    property color border:            dark ? darkBorder            : lightBorder
    property color borderInner:       dark ? darkBorderInner       : lightBorderInner
    property color textDark:          dark ? darkTextDark          : lightTextDark
    property color textMid:           dark ? darkTextMid           : lightTextMid
    property color textLight:         dark ? darkTextLight         : lightTextLight
    property color accent:            dark ? darkAccent            : lightAccent
    property color btnNormalBg:       dark ? darkBtnNormalBg       : lightBtnNormalBg
    property color btnNormalBorder:   dark ? darkBtnNormalBorder   : lightBtnNormalBorder
    property color btnPrimaryBg:      dark ? darkBtnPrimaryBg      : lightBtnPrimaryBg
    property color btnPrimaryBorder:  dark ? darkBtnPrimaryBorder  : lightBtnPrimaryBorder
    property color btnDangerBg:       dark ? darkBtnDangerBg       : lightBtnDangerBg
    property color btnDangerBorder:   dark ? darkBtnDangerBorder   : lightBtnDangerBorder
    property color btnDangerText:     dark ? "#FFA6A6"             : "#8B5A5A"

    // ========== 新增：现代按钮层级颜色 (Primary/Secondary/Ghost) ==========
    // Primary (主要操作)
    readonly property color primaryBtnBgLight:      "#4F46E5"
    readonly property color primaryBtnHoverLight:    "#4338CA"
    readonly property color primaryBtnPressedLight:  "#3730A3"
    readonly property color primaryBtnTextLight:     "#FFFFFF"
    readonly property color primaryBtnBgDark:        "#2DD4BF"
    readonly property color primaryBtnHoverDark:      "#40E0D0"
    readonly property color primaryBtnPressedDark:    "#20C0A0"
    readonly property color primaryBtnTextDark:      "#0F172A"

    property color primaryBtnBg:       dark ? primaryBtnBgDark       : primaryBtnBgLight
    property color primaryBtnHover:    dark ? primaryBtnHoverDark    : primaryBtnHoverLight
    property color primaryBtnPressed:  dark ? primaryBtnPressedDark  : primaryBtnPressedLight
    property color primaryBtnText:     dark ? primaryBtnTextDark     : primaryBtnTextLight

    // Secondary (次要操作)
    readonly property color secondaryBtnBgLight:         "#F3F4F6"
    readonly property color secondaryBtnBorderLight:     "#E5E7EB"
    readonly property color secondaryBtnHoverBgLight:    "#E5E7EB"
    readonly property color secondaryBtnHoverBorderLight:"#D1D5DB"
    readonly property color secondaryBtnTextLight:       "#1F2937"
    readonly property color secondaryBtnBgDark:          "#3A3A55"
    readonly property color secondaryBtnBorderDark:      "#4A4A6A"
    readonly property color secondaryBtnHoverBgDark:     "#4A4A6A"
    readonly property color secondaryBtnHoverBorderDark: "#5A5A7A"
    readonly property color secondaryBtnTextDark:        "#F1F1F6"

    property color secondaryBtnBg:          dark ? secondaryBtnBgDark          : secondaryBtnBgLight
    property color secondaryBtnBorder:      dark ? secondaryBtnBorderDark      : secondaryBtnBorderLight
    property color secondaryBtnHoverBg:     dark ? secondaryBtnHoverBgDark     : secondaryBtnHoverBgLight
    property color secondaryBtnHoverBorder: dark ? secondaryBtnHoverBorderDark : secondaryBtnHoverBorderLight
    property color secondaryBtnText:        dark ? secondaryBtnTextDark        : secondaryBtnTextLight

    // Ghost (低调操作)
    readonly property color ghostBtnTextLight:    "#4B5563"
    readonly property color ghostBtnHoverBgLight: "#F3F4F6"
    readonly property color ghostBtnTextDark:     "#C0C0E0"
    readonly property color ghostBtnHoverBgDark:  Qt.rgba(1,1,1,0.08)

    property color ghostBtnText:    dark ? ghostBtnTextDark    : ghostBtnTextLight
    property color ghostBtnHoverBg: dark ? ghostBtnHoverBgDark : ghostBtnHoverBgLight

    // ========== 颜色动画 ==========
    Behavior on bg          { ColorAnimation { duration: 300 } }
    Behavior on titleBg     { ColorAnimation { duration: 300 } }
    Behavior on sidebarBg   { ColorAnimation { duration: 300 } }
    Behavior on surface     { ColorAnimation { duration: 300 } }
    Behavior on border      { ColorAnimation { duration: 300 } }
    Behavior on borderInner { ColorAnimation { duration: 300 } }
    Behavior on textDark    { ColorAnimation { duration: 300 } }
    Behavior on textMid     { ColorAnimation { duration: 300 } }
    Behavior on textLight   { ColorAnimation { duration: 300 } }
    Behavior on accent      { ColorAnimation { duration: 300 } }
    Behavior on btnNormalBg     { ColorAnimation { duration: 300 } }
    Behavior on btnNormalBorder { ColorAnimation { duration: 300 } }
    Behavior on btnPrimaryBg    { ColorAnimation { duration: 300 } }
    Behavior on btnPrimaryBorder{ ColorAnimation { duration: 300 } }
    Behavior on btnDangerBg     { ColorAnimation { duration: 300 } }
    Behavior on btnDangerBorder { ColorAnimation { duration: 300 } }
    Behavior on btnDangerText   { ColorAnimation { duration: 300 } }

    Behavior on primaryBtnBg      { ColorAnimation { duration: 200 } }
    Behavior on primaryBtnHover   { ColorAnimation { duration: 200 } }
    Behavior on primaryBtnPressed { ColorAnimation { duration: 200 } }
    Behavior on primaryBtnText    { ColorAnimation { duration: 200 } }
    Behavior on secondaryBtnBg          { ColorAnimation { duration: 200 } }
    Behavior on secondaryBtnBorder      { ColorAnimation { duration: 200 } }
    Behavior on secondaryBtnHoverBg     { ColorAnimation { duration: 200 } }
    Behavior on secondaryBtnHoverBorder { ColorAnimation { duration: 200 } }
    Behavior on secondaryBtnText        { ColorAnimation { duration: 200 } }
    Behavior on ghostBtnText     { ColorAnimation { duration: 200 } }
    Behavior on ghostBtnHoverBg  { ColorAnimation { duration: 200 } }
}