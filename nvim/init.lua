-- 1. Sync Neovim copy/paste with your system clipboard
vim.opt.clipboard = "unnamedplus"

-- 2. Enable modern Neovim native autocompletion (Requires Neovim 0.12+)
if vim.fn.exists("+autocomplete") == 1 then vim.o.autocomplete = true end

-- 3. Load latest core UI 
local has_ui2, ui2 = pcall(require, "vim._core.ui2")
if has_ui2 then ui2.enable({}) end

-- 4. Load Modules
require("options")
require("keymaps")
require("commands")
require("pack")
require("theme")
