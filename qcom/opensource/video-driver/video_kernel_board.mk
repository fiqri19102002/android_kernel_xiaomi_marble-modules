# SPDX-License-Identifier: GPL-2.0-only
TARGET_VIDC_ENABLE := false
ifeq ($(TARGET_KERNEL_DLKM_DISABLE), true)
	ifeq ($(TARGET_KERNEL_DLKM_VIDEO_OVERRIDE), true)
		TARGET_VIDC_ENABLE := true
	endif
else
TARGET_VIDC_ENABLE := true
endif

# Build video kernel driver
ifeq ($(TARGET_VIDC_ENABLE),true)
ifneq ($(TARGET_BOARD_AUTO),true)
ifneq (,$(call is-board-platform-in-list2, $(TARGET_BOARD_PLATFORM)))
BOARD_VENDOR_KERNEL_MODULES += $(KERNEL_MODULES_OUT)/msm_video.ko

BUILD_VIDEO_TECHPACK_SOURCE := true
endif
endif
endif
