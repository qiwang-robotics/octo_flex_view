# 查找OctoFlexView库
#
# 结果变量:
#  OctoFlexView_FOUND        - 系统中找到了库
#  OctoFlexView_INCLUDE_DIRS - 库的头文件目录
#  OctoFlexView_LIBRARIES    - 链接库
#  OctoFlexView_VERSION      - 库的版本
#
# 本模块会定义以下导入的目标:
#  octo_flex::octo_flex_view - 主目标，使用find_package找到库后可以直接链接此目标

include(FindPackageHandleStandardArgs)

# 搜索路径
set(_OctoFlexView_SEARCH_PATHS
    ${OctoFlexView_DIR}
    $ENV{OctoFlexView_DIR}
    /usr/local
    /usr
    /opt/local
    /opt
)

# 查找头文件
find_path(OctoFlexView_INCLUDE_DIR
    NAMES octo_flex_viewer.h
    PATHS ${_OctoFlexView_SEARCH_PATHS}
    PATH_SUFFIXES include
)

# 查找库文件
find_library(OctoFlexView_LIBRARY
    NAMES octo_flex_view
    PATHS ${_OctoFlexView_SEARCH_PATHS}
    PATH_SUFFIXES lib lib64
)

# 如果找到了库，设置版本信息
if(OctoFlexView_INCLUDE_DIR AND OctoFlexView_LIBRARY)
    # 使用项目版本（如果需要从头文件提取，可以从 octo_flex_viewer.h 读取）
    set(OctoFlexView_VERSION "1.0.0")
endif()

# 处理查找结果
find_package_handle_standard_args(OctoFlexView
    REQUIRED_VARS OctoFlexView_LIBRARY OctoFlexView_INCLUDE_DIR
    VERSION_VAR OctoFlexView_VERSION
)

# 设置输出变量
if(OctoFlexView_FOUND)
    set(OctoFlexView_LIBRARIES ${OctoFlexView_LIBRARY})
    set(OctoFlexView_INCLUDE_DIRS ${OctoFlexView_INCLUDE_DIR})
    
    # 如果尚未定义导入的目标，则创建一个
    if(NOT TARGET octo_flex::octo_flex_view)
        add_library(octo_flex::octo_flex_view UNKNOWN IMPORTED)
        set_target_properties(octo_flex::octo_flex_view PROPERTIES
            IMPORTED_LOCATION "${OctoFlexView_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${OctoFlexView_INCLUDE_DIR}"
        )
    endif()
endif()

# 隐藏内部变量
mark_as_advanced(
    OctoFlexView_INCLUDE_DIR
    OctoFlexView_LIBRARY
) 