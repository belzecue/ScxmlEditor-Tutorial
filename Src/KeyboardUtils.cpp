/***********************************************************************************
	BSD 3-Clause License

	Copyright (c) 2018, https://github.com/alexzhornyak
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice, this
	  list of conditions and the following disclaimer.

	* Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.

	* Neither the name of the copyright holder nor the names of its
	  contributors may be used to endorse or promote products derived from
	  this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
	FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
	OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************************/

#include <vcl.h>
#pragma hdrstop

#include "KeyboardUtils.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace Keyboardutils {

	bool __fastcall IsControlPressed(void) {
		return(GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
	}

	bool __fastcall IsShiftPressed(void) {
		return(GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
	}

	bool __fastcall IsAltPressed(void) {
		return(GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
	}

	bool __fastcall IsAnyKeyOrMousePressed() {
		unsigned char chKeys[256] = {
			0
		};
		if (!GetKeyboardState(chKeys))
			return false;

		for (int i = 0; i < 256; i++) {
			if ((chKeys[i] & 0x80) != 0)
				return true;
		}

		return false;
	}

} // Keyboardutils
