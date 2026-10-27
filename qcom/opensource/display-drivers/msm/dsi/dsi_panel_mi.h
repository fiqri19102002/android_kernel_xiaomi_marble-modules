/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2026 The LineageOS Project
 */

#ifndef _DSI_PANEL_MI_H_
#define _DSI_PANEL_MI_H_

#include <linux/types.h>

#include "dsi_defs.h"

struct dsi_panel;

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
 */
struct dsi_panel_mi {
	u32 last_bl_lvl;
	bool doze_nolp;
	u32 doze_hbm_threshold;
	bool doze;
	enum dsi_panel_mi_doze doze_active;
};

void dsi_panel_mi_parse(struct dsi_panel *panel);

bool dsi_panel_mi_set_backlight(struct dsi_panel *panel, u32 *bl_lvl);

void dsi_panel_mi_reset_state(struct dsi_panel *panel);

bool dsi_panel_mi_has_doze(struct dsi_panel *panel);

int dsi_panel_mi_set_doze(struct dsi_panel *panel, bool doze);

#endif /* _DSI_PANEL_MI_H_ */
