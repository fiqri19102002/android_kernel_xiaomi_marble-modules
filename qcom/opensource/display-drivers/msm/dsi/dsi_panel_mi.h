/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2026 The LineageOS Project
 */

#ifndef _DSI_PANEL_MI_H_
#define _DSI_PANEL_MI_H_

#include <linux/types.h>

#include "dsi_defs.h"

struct dsi_panel;
struct dsi_display_mode;

enum dsi_panel_mi_doze {
	DSI_PANEL_MI_DOZE_OFF,
	DSI_PANEL_MI_DOZE_LBM,
	DSI_PANEL_MI_DOZE_HBM,
};

/**
 * struct dsi_panel_mi - Xiaomi specific panel state
 * @last_bl_lvl:  Last backlight level requested by the display driver.
 * @doze_nolp:    Leave doze through the doze HBM/LBM specific NOLP commands.
 * @doze_hbm_threshold: Doze backlight levels above this select doze HBM.
 * @doze:         Panel is in a low power mode.
 * @doze_active:  Doze brightness mode currently set on the panel.
 * @esd_irq:      Interrupt of the panel error flag, 0 if not available.
 * @esd_irq_flags: Trigger flags of @esd_irq.
 * @esd_irq_enabled: @esd_irq is enabled.
 * @esd_ignore_in_doze: Ignore panel errors while in doze.
 * @bl_sync_threshold: DBVs above this need the BL sync commands, 0 if unused.
 * @bl_sync:      BL sync is enabled.
 * @power_off_delay_ms: Delays after disabling the panel enable GPIO and
 *                after resetting the panel while powering off.
 * @doze_cmd_fps: Refresh rate the doze commands are made for, 0 for any.
 * @doze_fallback_dbv: HBM and LBM doze DBVs written at other refresh rates.
 * @dfps_vfp:     <fps vfp> pairs overriding the dynamic fps front porches.
 * @dfps_vfp_count: Number of u32 in @dfps_vfp.
 */
struct dsi_panel_mi {
	u32 last_bl_lvl;
	bool doze_nolp;
	u32 doze_hbm_threshold;
	bool doze;
	enum dsi_panel_mi_doze doze_active;

	int esd_irq;
	u32 esd_irq_flags;
	bool esd_irq_enabled;
	bool esd_ignore_in_doze;

	u32 bl_sync_threshold;
	bool bl_sync;

	u32 power_off_delay_ms[2];

	u32 doze_cmd_fps;
	u32 doze_fallback_dbv[2];

	u32 dfps_vfp[8];
	int dfps_vfp_count;
};

void dsi_panel_mi_parse(struct dsi_panel *panel);

bool dsi_panel_mi_set_backlight(struct dsi_panel *panel, u32 *bl_lvl);

void dsi_panel_mi_reset_state(struct dsi_panel *panel);

bool dsi_panel_mi_has_doze(struct dsi_panel *panel);

int dsi_panel_mi_set_doze(struct dsi_panel *panel, bool doze);

void dsi_panel_mi_esd_irq_ctrl(struct dsi_panel *panel, bool enable);

bool dsi_panel_mi_has_fps_gamma(struct dsi_panel *panel);

void dsi_panel_mi_fps_gamma(struct dsi_panel *panel);

void dsi_panel_mi_override_dfps_vfp(struct dsi_panel *panel,
		struct dsi_display_mode *mode);

#endif /* _DSI_PANEL_MI_H_ */
