-- Chromium OAuth windows start as "Untitled" and acquire a title later.
-- Static float rules miss this. Target the event's window, not the focused one.
local browsers = { chromium = true, ["chromium-browser"] = true,
                   ["google-chrome"] = true, ["google-chrome-stable"] = true }
local handled = {}
local function floatGoogleLogin(window)
    if not window or not window.mapped or not browsers[window.class:lower()] then return end
    if handled[window.stable_id] then return end
    local initial = window.initial_title or ""
    local popupStart = initial == "" or initial == "Untitled" or initial:find("Untitled - ", 1, true) == 1
    local title = window.title or ""
    local googleLogin = title:find("Google Accounts", 1, true) or title:find("Sign in with Google", 1, true)
    if not popupStart or not googleLogin then return end
    handled[window.stable_id] = true
    if window.floating then return end -- Preserve the user's existing placement.
    local monitor = window.monitor
    local width, height = 560, 680
    if monitor then
        width = math.min(width, math.floor(monitor.width / monitor.scale) - 40)
        height = math.min(height, math.floor(monitor.height / monitor.scale) - 100)
    end
    hl.dispatch(hl.dsp.window.float({ window = window, action = "set" }))
    hl.dispatch(hl.dsp.window.resize({ window = window, x = width, y = height, relative = false }))
    hl.dispatch(hl.dsp.window.center({ window = window }))
end
hl.on("window.open", floatGoogleLogin)
hl.on("window.title", floatGoogleLogin)
hl.on("window.close", function(window)
    if window then handled[window.stable_id] = nil end
end)
-- Repair already-open login popups when the config reloads, too.
for _, window in ipairs(hl.get_windows()) do floatGoogleLogin(window) end
