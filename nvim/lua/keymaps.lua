-- native undotree
vim.g.mapleader=" "

-- native undotree
vim.keymap.set("n", "<leader>u", function()
    local ok = pcall(vim.cmd.packadd, "nvim.undotree")
    local has_undotree, undotree = pcall(require, "undotree")
    if ok and has_undotree then undotree.open()
    else vim.notify("Native undotree is unavailable in this Neovim version.", vim.log.levels.INFO) end
end, { desc = "Toggle Builtin Undotree" })
