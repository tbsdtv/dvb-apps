/*
  CA-ZAP utility

  Copyright (C) 2004, 2005 Manu Abraham (manu@kromtek.com)
  Copyright (C) 2006 Andrew de Quincey (adq_dvb@lidskialf.net)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of
  the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

#ifndef LLCI_H
#define LLCI_H

#include <stdint.h>

extern int llci_init();

extern int llci_cam_added(int cafd, uint8_t slot);

extern void llci_cam_removed();

extern void llci_poll();

extern void llci_shutdown();

#endif
