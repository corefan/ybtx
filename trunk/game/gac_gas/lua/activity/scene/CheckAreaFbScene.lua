--gac_gas_require "scene_mgr/SceneCfg"
--cfg_load "fb_game/AreaFb_Common"
--
--local function GetItemTbl(item)
--	local tbl = loadstring("return" .. item)()
--	return tbl
--end

--����AreaFbBoss_Server��
--local function LoadAreaFbBoss_Server()
--	for r, TblInfo in pairs(AreaFb_Common) do 
--		local BossName = GetItemTbl(TblInfo.BossName)
--		rawset(TblInfo, "BossName", BossName)	
--	end 
--end
--
--LoadAreaFbBoss_Server()