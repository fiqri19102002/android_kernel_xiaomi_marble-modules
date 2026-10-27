// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 The LineageOS Project
 */

#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/of_gpio.h>

#include "dsi_panel.h"
#include "dsi_panel_mi.h"

static void dsi_panel_mi_parse_esd(struct dsi_panel *panel)
{
	struct dsi_parser_utils *utils = &panel->utils;
	struct dsi_panel_mi *mi = &panel->mi;
	enum of_gpio_flags flags;
	int gpio, rc;

	gpio = of_get_named_gpio_flags(utils->data, "mi,esd-err-irq-gpio", 0,
			&flags);
	if (!gpio_is_valid(gpio))
		return;

	rc = devm_gpio_request_one(panel->parent, gpio, GPIOF_IN,
			"esd_err_irq_gpio");
	if (rc) {
		DSI_ERR("[%s] failed to request esd gpio %d, rc=%d\n",
				panel->name, gpio, rc);
		return;
	}

	rc = gpio_to_irq(gpio);
	if (rc < 0)
		return;

	mi->esd_irq = rc;
	mi->esd_irq_flags = flags;
	mi->esd_ignore_in_doze = utils->read_bool(utils->data,
			"mi,panel-ignore-esd-in-aod");
}

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

	utils->read_u32(utils->data, "mi,bl-sync-threshold",
			&mi->bl_sync_threshold);
	utils->read_u32_array(utils->data, "mi,power-off-delay-ms",
			mi->power_off_delay_ms, ARRAY_SIZE(mi->power_off_delay_ms));
	utils->read_u32(utils->data, "mi,doze-cmd-fps", &mi->doze_cmd_fps);
	utils->read_u32_array(utils->data, "mi,doze-fallback-dbv",
			mi->doze_fallback_dbv, ARRAY_SIZE(mi->doze_fallback_dbv));

	dsi_panel_mi_parse_esd(panel);
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

	if (mi->bl_sync_threshold &&
	    (*bl_lvl > mi->bl_sync_threshold) != mi->bl_sync) {
		mi->bl_sync = !mi->bl_sync;
		dsi_panel_tx_cmd_set(panel, mi->bl_sync ?
				DSI_CMD_SET_MI_BL_SYNC_ON :
				DSI_CMD_SET_MI_BL_SYNC_OFF);
	}

	return false;
}

/* Called whenever the panel itself leaves doze */
void dsi_panel_mi_reset_state(struct dsi_panel *panel)
{
	panel->mi.doze = false;
	panel->mi.doze_active = DSI_PANEL_MI_DOZE_OFF;
	panel->mi.bl_sync = false;
}

bool dsi_panel_mi_has_doze(struct dsi_panel *panel)
{
	if (!panel->cur_mode || !panel->cur_mode->priv_info)
		return false;

	return panel->cur_mode->priv_info->cmd_sets[DSI_CMD_SET_MI_DOZE_LBM].count;
}

static int dsi_panel_mi_set_dbv_lp(struct dsi_panel *panel, u32 dbv)
{
	struct mipi_dsi_device *dsi = &panel->mipi_device;
	unsigned long mode_flags = dsi->mode_flags;
	int rc;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;
	rc = mipi_dsi_dcs_set_display_brightness(dsi, dbv);
	dsi->mode_flags = mode_flags;

	return rc < 0 ? rc : 0;
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

	if (target != DSI_PANEL_MI_DOZE_OFF && mi->doze_cmd_fps &&
	    panel->cur_mode->timing.refresh_rate != mi->doze_cmd_fps)
		rc = dsi_panel_mi_set_dbv_lp(panel, mi->doze_fallback_dbv[
				target == DSI_PANEL_MI_DOZE_HBM ? 0 : 1]);
	else
		rc = dsi_panel_tx_cmd_set(panel, type);
	if (rc)
		DSI_ERR("[%s] failed to set doze mode %d, rc=%d\n",
				panel->name, target, rc);

	mi->doze_active = target;

	return rc;
}

void dsi_panel_mi_esd_irq_ctrl(struct dsi_panel *panel, bool enable)
{
	struct dsi_panel_mi *mi = &panel->mi;

	if (!mi->esd_irq || mi->esd_irq_enabled == enable)
		return;

	if (enable) {
		enable_irq_wake(mi->esd_irq);
		enable_irq(mi->esd_irq);
	} else {
		disable_irq_wake(mi->esd_irq);
		disable_irq_nosync(mi->esd_irq);
	}

	mi->esd_irq_enabled = enable;
}

static enum dsi_cmd_set_type dsi_panel_mi_fps_gamma_type(struct dsi_panel *panel)
{
	switch (panel->cur_mode->timing.refresh_rate) {
	case 60:
		return DSI_CMD_SET_MI_FPS_60_GAMMA;
	case 90:
		return DSI_CMD_SET_MI_FPS_90_GAMMA;
	case 120:
		return DSI_CMD_SET_MI_FPS_120_GAMMA;
	default:
		return DSI_CMD_SET_MAX;
	}
}

bool dsi_panel_mi_has_fps_gamma(struct dsi_panel *panel)
{
	enum dsi_cmd_set_type type;

	if (!panel->cur_mode || !panel->cur_mode->priv_info)
		return false;

	type = dsi_panel_mi_fps_gamma_type(panel);

	return type != DSI_CMD_SET_MAX &&
			panel->cur_mode->priv_info->cmd_sets[type].count;
}

/* Called with panel_lock held, after switching the video mode refresh rate */
void dsi_panel_mi_fps_gamma(struct dsi_panel *panel)
{
	if (!panel->panel_initialized || !dsi_panel_mi_has_fps_gamma(panel))
		return;

	if (dsi_panel_tx_cmd_set(panel, dsi_panel_mi_fps_gamma_type(panel)))
		DSI_ERR("[%s] failed to switch fps gamma\n", panel->name);
}
