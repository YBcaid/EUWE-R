TEMPLATE = subdirs

SUBDIRS += \
    app \
    common \
    core \
    modules \
    tests \
    ui

# 依赖顺序
core.depends = common
modules.depends = common core
ui.depends = common core modules
app.depends = common core modules ui
tests.depends = common core modules