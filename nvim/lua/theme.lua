-- Ensures font icons and colors render perfectly inside Kitty
vim.opt.termguicolors = true

-- Force Neovim to use Kitty's 0.80 transparent background
local function make_transparent()
  local groups = { "Normal", "NormalNC", "SignColumn", "LineNr", "StatusLine", "EndOfBuffer" }
  for _, group in ipairs(groups) do
    vim.api.nvim_set_hl(0, group, { bg = "NONE", ctermbg = "NONE" })
  end
end

make_transparent()
vim.api.nvim_create_autocmd("ColorScheme", { callback = make_transparent })


