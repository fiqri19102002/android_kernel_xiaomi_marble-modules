// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 The LineageOS Project
 */

#include "dsi_panel.h"
#include "dsi_panel_mi.h"

void dsi_panel_mi_parse(struct dsi_panel *panel)
{
	struct dsi_parser_utils *utils = &panel->utils;
	struct dsi_panel_mi *mi = &panel->mi;
	u32 hbm_dbv, lbm_dbv;

	mi->doze_nolp = utils->read_bool(utils->data,
			"mi,aod-nolp-command-enabled");

	if (!utils->read_u32(utils->data, "mi,doze-hbm-dbv-level", &hbm_dbv) &&
	    !utils->read_u32(utils->data, "mi,doze-lbm-dbv-level", &lbm_dbv))
		mi->doze_hbm_threshold = (hbm_dbv + lbm_dbv) / 2;
	else
		mi->doze_hbm_threshold = 100;
}

bool dsi_panel_mi_set_backlight(struct dsi_panel *panel, u32 *bl_lvl)
{
	struct dsi_panel_mi *mi = &panel->mi;

	mi->last_bl_lvl = *bl_lvl;

	/* Doze brightness is part of the doze commands */
	if (mi->doze && dsi_panel_mi_has_doze(panel)) {
		if (*bl_lvl)
			dsi_panel_mi_set_doze(panel, true);
		return true;
	}

	return false;
}

/* Called whenever the panel itself leaves doze */
void dsi_panel_mi_reset_state(struct dsi_panel *panel)
{
	panel->mi.doze = false;
	panel->mi.doze_active = DSI_PANEL_MI_DOZE_OFF;
}

bool dsi_panel_mi_has_doze(struct dsi_panel *panel)
{
	if (!panel->cur_mode || !panel->cur_mode->priv_info)
		return false;

	return panel->cur_mode->priv_info->cmd_sets[DSI_CMD_SET_MI_DOZE_LBM].count;
}

int dsi_panel_mi_set_doze(struct dsi_panel *panel, bool doze)
{
	struct dsi_panel_mi *mi = &panel->mi;
	enum dsi_panel_mi_doze target = DSI_PANEL_MI_DOZE_OFF;
	enum dsi_cmd_set_type type;
	int rc;

	mi->doze = doze;

	if (doze)
		target = mi->last_bl_lvl > mi->doze_hbm_threshold ?
				DSI_PANEL_MI_DOZE_HBM : DSI_PANEL_MI_DOZE_LBM;

	if (target == mi->doze_active)
		return 0;

	switch (target) {
	case DSI_PANEL_MI_DOZE_HBM:
		type = DSI_CMD_SET_MI_DOZE_HBM;
		break;
	case DSI_PANEL_MI_DOZE_LBM:
		type = DSI_CMD_SET_MI_DOZE_LBM;
		break;
	default:
		type = DSI_CMD_SET_NOLP;
		if (mi->doze_nolp)
			type = mi->doze_active == DSI_PANEL_MI_DOZE_HBM ?
					DSI_CMD_SET_MI_DOZE_HBM_NOLP :
					DSI_CMD_SET_MI_DOZE_LBM_NOLP;
		break;
	}

	rc = dsi_panel_tx_cmd_set(panel, type);
	if (rc)
		DSI_ERR("[%s] failed to set doze mode %d, rc=%d\n",
				panel->name, target, rc);

	mi->doze_active = target;

	return rc;
}
