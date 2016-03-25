#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/MAP2N
	NAME:=MAP2N
	PACKAGES:=kmod-usb-core kmod-usb2
endef

define Profile/MAP2N/Description
	Package set optimized for the GL-Connect MAP2N.
endef

$(eval $(call Profile,MAP2N))

