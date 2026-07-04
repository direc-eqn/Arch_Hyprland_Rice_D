-- 1. Sync Neovim copy/paste with your system clipboard
vim.opt.clipboard = "unnamedplus"

-- 2. Enable modern Neovim native autocompletion (Requires Neovim 0.12+)
vim.o.autocomplete = true

-- 3. Load latest core UI 
require("vim._core.ui2").enable({})

-- 4. Load Modules
require("options")
require("keymaps")
require("commands")
require("pack")
