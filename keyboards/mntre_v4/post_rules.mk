# Reform system controller over UART: power keys, battery and status pages,
# Circle long-press power on. Build the standalone keyboard (MNT's
# KBD_MODE_STANDALONE) without it:
#   make mntre_v4:default REFORM4_SYSCTL=no
REFORM4_SYSCTL ?= yes

ifeq ($(strip $(REFORM4_SYSCTL)), yes)
    SRC += sysctl.c
    UART_DRIVER_REQUIRED = yes
    OPT_DEFS += -DREFORM4_SYSCTL_ENABLE
endif

ifeq ($(strip $(OLED_ENABLE)), yes)
    SRC += reform4_oled.c
endif
