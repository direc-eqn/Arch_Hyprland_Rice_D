-- Run from the repository root: lua tests/test_browser_dialogs.lua
local events, calls = {}, {}
local function dispatcher(name)
    return function(args) return { name = name, args = args } end
end
hl = {
    on = function(name, callback) events[name] = callback end,
    get_windows = function() return {} end,
    dispatch = function(command) table.insert(calls, command) end,
    dsp = { window = { float = dispatcher("float"), resize = dispatcher("resize"), center = dispatcher("center") } },
}
dofile("hypr/browser-dialogs.lua")
local function window(id, initial, title, class)
    return { stable_id = id, initial_title = initial, title = title,
             class = class or "chromium", mapped = true, floating = false,
             monitor = { width = 1920, height = 1080, scale = 1.2 } }
end
local popup = window(1, "Untitled - Chromium", "Untitled - Chromium")
events["window.open"](popup)
assert(#calls == 0, "Do not float all untitled windows")
popup.title = "Sign in - Google Accounts - Chromium"
events["window.title"](popup)
assert(#calls == 3 and calls[1].name == "float", "Late title must float the popup")
for _, call in ipairs(calls) do assert(call.args.window == popup, "Do not act on the focused window") end
assert(calls[2].args.x == 560 and calls[2].args.y == 680)
events["window.title"](popup)
assert(#calls == 3, "Do not reset user placement on repeated title events")
events["window.open"](window(2, "New Tab - Chromium", popup.title))
events["window.title"](window(3, "Untitled - Chromium", popup.title, "kitty"))
events["window.title"](window(4, "Untitled - Chromium", "An unrelated popup"))
assert(#calls == 3, "Ordinary tabs and other apps must stay untouched")
local chrome = window(5, "Untitled - Google Chrome", "Sign in with Google - Google Chrome", "google-chrome")
events["window.open"](chrome)
assert(#calls == 6, "Handle Chrome when title is already known on open")
local small = window(6, "", popup.title)
small.monitor = { width = 800, height = 600, scale = 1 }
events["window.open"](small)
assert(calls[8].args.y == 500, "Fit smaller screens")
local positioned = window(7, "Untitled", popup.title)
positioned.floating = true
events["window.open"](positioned)
assert(#calls == 9, "Preserve existing floating window placement")
print("Browser popup regression checks passed")
