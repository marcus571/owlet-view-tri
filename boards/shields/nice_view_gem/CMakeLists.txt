# ============================================================
# nice_view_gem CMakeLists.txt
# 路径：boards/shields/nice_view_gem/CMakeLists.txt
#
# 2026-11-06 终极修复版：
#   移除了 assets/pixel_operator_mono.c，
#   因为字体定义已经在 custom_status_screen.c 中包含，导致重复定义。
# ============================================================

if(CONFIG_ZMK_DISPLAY)
  target_sources(app PRIVATE
    custom_status_screen.c       # 核心入口
    widgets/screen.c
    widgets/battery.c
    widgets/output.c
    widgets/layer.c
    widgets/util.c
    assets/images.c
  )
endif()
