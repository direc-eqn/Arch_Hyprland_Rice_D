-- Hyprland 0.56+ (Lua). Start here; Waybar's appearance lives in waybar/style.css.
-- Personal overrides can go in ~/.config/hypr/local.lua (ignored by Git).
-- Shortcut guide: Super + / or the ? button on Waybar.

-- 1. Preferences ---------------------------------------------------------------
local home = os.getenv("HOME")
local configHome = os.getenv("XDG_CONFIG_HOME") or (home .. "/.config")
local mainMod = "SUPER"
local terminal = "kitty"
local fileManager = "kitty -e yazi"
local menu = "rofi -show drun -show-icons"
local laptopOutput = "eDP-1" -- Find output names with: hyprctl monitors
local laptopScale = 1.2

-- 2. Displays and laptop lid ----------------------------------------------------
hl.monitor({ output = "", mode = "preferred", position = "auto", scale = "auto" })
local function enableLaptop()
    hl.monitor({ output = laptopOutput, mode = "preferred", position = "auto",
                 scale = laptopScale, disabled = false })
end
local function closeLid()
    -- Never disable the only display. With an external display, keep working there.
    for _, monitor in ipairs(hl.get_monitors()) do
        if monitor.name ~= laptopOutput then
            hl.monitor({ output = laptopOutput, disabled = true })
            return
        end
    end
end
enableLaptop()
hl.bind("switch:on:Lid Switch", closeLid)
hl.bind("switch:off:Lid Switch", enableLaptop)

-- 3. Environment and startup ---------------------------------------------------
hl.env("XCURSOR_SIZE", "24")
hl.env("HYPRCURSOR_SIZE", "24")
hl.env("QT_QPA_PLATFORMTHEME", "qt6ct")
-- This laptop uses NVIDIA. Remove these two lines on systems without NVIDIA.
hl.env("LIBVA_DRIVER_NAME", "nvidia")
hl.env("__GLX_VENDOR_LIBRARY_NAME", "nvidia")
hl.env("SSH_AUTH_SOCK", "$XDG_RUNTIME_DIR/ssh-agent.socket")
hl.env("HYPRSHOT_DIR", home .. "/Pictures/Screenshots")

hl.on("hyprland.start", function()
    hl.exec_cmd("dbus-update-activation-environment --systemd WAYLAND_DISPLAY XDG_CURRENT_DESKTOP")
    hl.exec_cmd("systemctl --user start hyprpolkitagent")
    for _, program in ipairs({ "blueman-applet", "nm-applet", "hyprpaper", "swaync", "hypridle", "waybar" }) do
        hl.exec_cmd("command -v " .. program .. " >/dev/null 2>&1 && " .. program)
    end
    -- To enable the schedule in hyprsunset.conf, add "hyprsunset" to the list above.
    local handle = io.popen("cat /proc/acpi/button/lid/*/state 2>/dev/null")
    if handle then
        local state = handle:read("*a")
        handle:close()
        if state and state:find("closed") then closeLid() end
    end
end)

-- 4. Appearance and input ------------------------------------------------------
-- Teal accents match Waybar. Blur and shadows stay off to limit GPU use.
hl.config({
    general = {
        gaps_in = 2,
        gaps_out = 10,
        border_size = 2,
        col = { active_border = "rgba(7dd3c7ff)", inactive_border = "rgba(354553aa)" },
        resize_on_border = true,
        allow_tearing = false,
        layout = "dwindle",
    },
    decoration = {
        rounding = 10,
        rounding_power = 2,
        shadow = { enabled = false },
        blur = { enabled = false },
    },
    animations = { enabled = true },
    dwindle = { preserve_split = true },
    misc = {
        force_default_wallpaper = -1,
        disable_hyprland_logo = true,
        vrr = 1, -- Adaptive sync where supported by the monitor.
    },
    input = {
        kb_layout = "us",
        follow_mouse = 1,
        sensitivity = 0,
        touchpad = { natural_scroll = false, scroll_factor = 0.4 },
    },
    xwayland = { force_zero_scaling = true },
})
hl.gesture({ fingers = 3, direction = "horizontal", action = "workspace" })

-- 5. Keyboard and mouse --------------------------------------------------------
hl.bind(mainMod .. " + Q", hl.dsp.exec_cmd(terminal))
hl.bind(mainMod .. " + E", hl.dsp.exec_cmd(fileManager))
hl.bind(mainMod .. " + R", hl.dsp.exec_cmd(menu))
hl.bind(mainMod .. " + C", hl.dsp.window.close())
hl.bind(mainMod .. " + V", hl.dsp.window.float({ action = "toggle" }))
hl.bind(mainMod .. " + F", hl.dsp.window.fullscreen({ action = "toggle" }))
hl.bind(mainMod .. " + P", hl.dsp.window.pseudo())
hl.bind(mainMod .. " + J", hl.dsp.layout("togglesplit"))
hl.bind(mainMod .. " + M", hl.dsp.exec_cmd("command -v hyprshutdown >/dev/null 2>&1 && hyprshutdown || hyprctl dispatch 'hl.dsp.exit()'"))

hl.bind(mainMod .. " + mouse:272", hl.dsp.window.drag(), { mouse = true })
hl.bind(mainMod .. " + mouse:273", hl.dsp.window.resize(), { mouse = true })
for _, direction in ipairs({ "left", "right", "up", "down" }) do
    hl.bind(mainMod .. " + " .. direction, hl.dsp.focus({ direction = direction }))
end
for workspace = 1, 10 do
    local key = workspace % 10
    hl.bind(mainMod .. " + " .. key, hl.dsp.focus({ workspace = workspace }))
    hl.bind(mainMod .. " + SHIFT + " .. key, hl.dsp.window.move({ workspace = workspace }))
end
hl.bind(mainMod .. " + mouse_down", hl.dsp.focus({ workspace = "e+1" }))
hl.bind(mainMod .. " + mouse_up", hl.dsp.focus({ workspace = "e-1" }))
hl.bind(mainMod .. " + SHIFT + S", hl.dsp.exec_cmd("hyprshot -m region"))
hl.bind(mainMod .. " + L", hl.dsp.exec_cmd("pidof hyprlock || hyprlock"))
hl.bind(mainMod .. " + N", hl.dsp.exec_cmd("swaync-client -t -sw"))
hl.bind(mainMod .. " + SHIFT + N", hl.dsp.exec_cmd("swaync-client -d -sw"))
hl.bind(mainMod .. " + slash", hl.dsp.exec_cmd('"' .. configHome .. '/hypr/scripts/shortcuts.sh"'))
hl.bind(mainMod .. " + SHIFT + R", hl.dsp.exec_cmd("hyprctl reload && pkill -USR2 -x waybar"))

-- These work while locked; repeating volume keys are capped at 100%.
hl.bind("XF86AudioRaiseVolume", hl.dsp.exec_cmd("wpctl set-volume -l 1 @DEFAULT_AUDIO_SINK@ 5%+"), { locked = true, repeating = true })
hl.bind("XF86AudioLowerVolume", hl.dsp.exec_cmd("wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%-"), { locked = true, repeating = true })
hl.bind("XF86AudioMute", hl.dsp.exec_cmd("wpctl set-mute @DEFAULT_AUDIO_SINK@ toggle"), { locked = true })
hl.bind("XF86AudioMicMute", hl.dsp.exec_cmd("wpctl set-mute @DEFAULT_AUDIO_SOURCE@ toggle"), { locked = true })

-- 6. Window rules --------------------------------------------------------------
-- Modal dialogs float above the tiles; some apps need file-chooser fallbacks.
hl.window_rule({ name = "float-modal-dialogs", match = { modal = true }, float = true })
hl.window_rule({
    name = "float-file-chooser-portals",
    match = { class = "^(xdg-desktop-portal-gtk|xdg-desktop-portal-kde|org\\.freedesktop\\.impl\\.portal\\.desktop\\.kde)$" },
    float = true,
})
hl.window_rule({
    name = "float-file-chooser-dialogs",
    match = { initial_title = "^(Open File|Open Files|Open Folder|Save File|Save As|Save As\\.\\.\\.|Save As…|Select File|Select Files|Select Folder|Choose File|Choose Files|Choose Folder|File Upload)( - .*)?$" },
    float = true,
})
hl.window_rule({ name = "suppress-maximize-events", match = { class = ".*" }, suppress_event = "maximize" })
hl.window_rule({
    name = "fix-xwayland-drags",
    match = { class = "^$", title = "^$", xwayland = true, float = true, fullscreen = false, pin = false },
    no_focus = true,
})
hl.window_rule({ name = "move-hyprland-run", match = { class = "hyprland-run" }, move = "20 monitor_h-120", float = true })

-- 7. Optional machine-specific overrides --------------------------------------
local localPath = configHome .. "/hypr/local.lua"
local localFile = io.open(localPath, "r")
if localFile then
    localFile:close()
    dofile(localPath)
end
