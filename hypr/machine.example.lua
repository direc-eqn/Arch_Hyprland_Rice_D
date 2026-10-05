-- Copy to ~/.config/hypr/machine.lua if you want to configure a machine manually.
-- restore.py creates that ignored file automatically; edit it after installation.
return {
    laptop_output = "eDP-1", -- hyprctl monitors; omit this key on a desktop
    laptop_scale = "auto",  -- use 1.2 to reproduce the original laptop's scaling
    nvidia_env = false,     -- opt in only if your NVIDIA setup needs forced variables
    night_light = false,    -- true: Hyprland starts the bundled evening schedule
}
