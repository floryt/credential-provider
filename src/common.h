//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// This file contains some global variables that describe what our
// sample tile looks like.  For example, it defines what fields a tile has
// and which fields show in which states of LogonUI. This sample illustrates
// the use of each UI field type.

#pragma once
#include "helpers.h"

//BAR: the indexes of the fields in the array
// The indexes of each of the fields in our credential provider's tiles. Note that we're
// using each of the nine available field types here.
enum SAMPLE_FIELD_ID
{
	SFI_TILEIMAGE = 0,
	SFI_LABEL = 1,
	SFI_LARGE_TEXT = 2,
	SFI_PASSWORD = 3,
	SFI_SUBMIT_BUTTON = 4,
	SFI_LAUNCHWINDOW_LINK = 5,
	SFI_HIDECONTROLS_LINK = 6,
	SFI_FULLNAME_TEXT = 7,
	SFI_DISPLAYNAME_TEXT = 8,
	SFI_LOGONSTATUS_TEXT = 9,
	SFI_CHECKBOX = 10,
	SFI_EDIT_TEXT = 11,
	SFI_COMBOBOX = 12,
	SFI_NUM_FIELDS = 13,  // Note: if new fields are added, keep NUM_FIELDS last.  This is used as a count of the number of fields
};

// The first value indicates when the tile is displayed (selected, not selected)
// the second indicates things like whether the field is enabled, whether it has key focus, etc.
struct FIELD_STATE_PAIR
{
	CREDENTIAL_PROVIDER_FIELD_STATE cpfs;
	CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE cpfis;
};

// These two arrays are seperate because a credential provider might
// want to set up a credential with various combinations of field state pairs
// and field descriptors.

//BAR: initial show/hidden for fileds. you can change them during the code as shown in the function CommandLinkClicked.
// The field state value indicates whether the field is displayed
// in the selected tile, the deselected tile, or both.
// The Field interactive state indicates when
static const FIELD_STATE_PAIR s_rgFieldStatePairs[] =
{
	{ CPFS_DISPLAY_IN_BOTH,            CPFIS_NONE    },    // SFI_TILEIMAGE 
	{ CPFS_HIDDEN,                     CPFIS_NONE    },    // SFI_LABEL
	{ CPFS_DISPLAY_IN_BOTH,            CPFIS_NONE    },    // SFI_LARGE_TEXT
	{ CPFS_HIDDEN,   CPFIS_NONE },    // SFI_PASSWORD
	{ CPFS_HIDDEN,   CPFIS_NONE },    // SFI_SUBMIT_BUTTON
	{ CPFS_HIDDEN,   CPFIS_NONE    },    // SFI_LAUNCHWINDOW_LINK
	{ CPFS_DISPLAY_IN_SELECTED_TILE,   CPFIS_NONE    },    // SFI_HIDECONTROLS_LINK
	{ CPFS_HIDDEN, CPFIS_NONE },    // SFI_FULLNAME_TEXT
	{ CPFS_HIDDEN, CPFIS_NONE },    // SFI_DISPLAYNAME_TEXT
	{ CPFS_HIDDEN, CPFIS_NONE },    // SFI_LOGONSTATUS_TEXT
	{ CPFS_HIDDEN, CPFIS_NONE },    // SFI_CHECKBOX
	{ CPFS_DISPLAY_IN_SELECTED_TILE, CPFIS_FOCUSED },    // SFI_EDIT_TEXT
	{ CPFS_HIDDEN, CPFIS_NONE },    // SFI_COMBOBOX
};

//BAR: the NAMES and the tiles. I recommand to not earse any tiles at first because it can lead to many problems, and wil require many modifications in the entire code.
//STEVEN: here are the default text values
// Field descriptors for unlock and logon.
// The first field is the index of the field.
// The second is the type of the field.
// The third is the name of the field, NOT the value which will appear in the field.
static const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR s_rgCredProvFieldDescriptors[] =
{
	{ SFI_TILEIMAGE,         CPFT_TILE_IMAGE,    L"Image",                      CPFG_CREDENTIAL_PROVIDER_LOGO  },
	{ SFI_LABEL,             CPFT_SMALL_TEXT,    L"Tooltip",                    CPFG_CREDENTIAL_PROVIDER_LABEL },
	{ SFI_LARGE_TEXT,        CPFT_LARGE_TEXT,    L"Floryt Credential Provider"                                 }, //STEVEN: edit here did not effected the dll
	{ SFI_PASSWORD,          CPFT_PASSWORD_TEXT, L"Enter password here"                                        }, //STEVEN: edit here did take effect!!
	{ SFI_SUBMIT_BUTTON,     CPFT_SUBMIT_BUTTON, L"Submit"                                                     },
	{ SFI_LAUNCHWINDOW_LINK, CPFT_COMMAND_LINK,  L"Launch helper window"                                       },
	{ SFI_HIDECONTROLS_LINK, CPFT_COMMAND_LINK,  L"Hide additional controls"                                   },
	{ SFI_FULLNAME_TEXT,     CPFT_SMALL_TEXT,    L"Full name: "                                                },
	{ SFI_DISPLAYNAME_TEXT,  CPFT_SMALL_TEXT,    L"Display name: "                                             },
	{ SFI_LOGONSTATUS_TEXT,  CPFT_SMALL_TEXT,    L"Logon status: "                                             },
	{ SFI_CHECKBOX,          CPFT_CHECKBOX,      L"Connect as guest"                                           },
	{ SFI_EDIT_TEXT,         CPFT_EDIT_TEXT,     L"Enter your email"                                           },
	{ SFI_COMBOBOX,          CPFT_COMBOBOX,      L"Combobox"                                                   },
};

static const PWSTR s_rgComboBoxStrings[] =
{
	L"First",
	L"Second",
	L"Third",
};
