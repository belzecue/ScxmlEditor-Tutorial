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

#undef TextOut

#include "UnitSVGExporter.h"

#include <memory>

#include "Log4cpp_VCL.hpp"
#include <PNGImage.hpp>

/* https://github.com/TurboPack/PNGComponents */
/* Mozilla Public License 1.1 (MPL 1.1) */
#include "PNGFunctions.hpp"

#include <LMDXml.hpp>
#include "ImageUtils.h"
#include "UnitSettings.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

class TTeePanelAccess : public TCustomTeePanel {
public:
	__property Tecanvas::TCanvas3D* InternalCanvas3D = {
		read = InternalCanvas, write = InternalCanvas
	};
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
__fastcall TTreeSVGCanvasEx::TTreeSVGCanvasEx(const UnicodeString sUnitName, Classes::TStrings* AStrings) : TTreeSVGCanvas(AStrings),
UnitName(sUnitName) {
}

//---------------------------------------------------------------------------
Types::TRect __fastcall TTreeSVGCanvasEx::InitWindow(Graphics::TCanvas* DestCanvas, Tecanvas::TView3DOptions* A3DOptions,
	Graphics::TColor ABackColor, bool Is3D, const Types::TRect &UserRect) {

	const TRect AResultRect = TTreeSVGCanvas::InitWindow(DestCanvas, A3DOptions, ABackColor, Is3D, UserRect);

	Add("<!--This SVG file was automatically generated by ScxmlEditor[" + SettingsData->AppVersion + "] from [" + UnitName + "]-->");

	return AResultRect;
}

//---------------------------------------------------------------------------
System::UnicodeString __fastcall TTreeSVGCanvasEx::HeaderContents(void) {
	UnicodeString sSvgExtraParams = SettingsData->ExportSvgSettings->GetSvgParams();
	if (!sSvgExtraParams.IsEmpty()) {
		sSvgExtraParams = sSvgExtraParams + L"\n";
	}
	return TTreeSVGCanvas::HeaderContents() + sSvgExtraParams;
}

//---------------------------------------------------------------------------
void __fastcall TTreeSVGCanvasEx::Draw(int X, int Y, Graphics::TGraphic* Graphic) {
	if (Graphic) {
		const TRect ARect(X, Y, X + Graphic->Width, Y + Graphic->Height);

		std::auto_ptr<TMemoryStream>AOutStreamPtr(new TMemoryStream());

		if (TPngImage * APngImage = dynamic_cast<TPngImage*>(Graphic)) {
			APngImage->SaveToStream(AOutStreamPtr.get());
		}
		else {
			std::unique_ptr<TPngImage>APngImagePtr(new TPngImage);

			Pngfunctions::ConvertToPNG(Graphic, APngImagePtr.get());

			APngImagePtr->SaveToStream(AOutStreamPtr.get());
		}

		AOutStreamPtr->Position = 0;

		Add(L"<image");
		Add(L" preserveAspectRatio=\"none\"");
		Add(SVGRect(ARect));
		Add(" xlink:href=\"data:image/png;base64," + LMDBinToBase64(AOutStreamPtr->Memory, AOutStreamPtr->Size, MaxInt / 4) + L"\"");
		AddEnd(L"");
	}
}

//---------------------------------------------------------------------------
void __fastcall TTreeSVGCanvasEx::StretchDraw(const Types::TRect &Rect, Graphics::TGraphic* Graphic)/* overload */ {
	if (Graphic) {
		if (Rect.Width() == Graphic->Width && Rect.Height() == Graphic->Height) {
			this->Draw(Rect.left, Rect.top, Graphic);
		}
		else {
			std::auto_ptr<Graphics::TBitmap>ABitmapPtr(new Graphics::TBitmap());

			Imgutils::ResizeGraphicSmooth(ABitmapPtr.get(), Graphic, Rect.Width(), Rect.Height());

			this->Draw(Rect.left, Rect.top, ABitmapPtr.get());
		}
	}
}

//---------------------------------------------------------------------------
void __fastcall TTreeSVGCanvasEx::RotateLabel(int x, int y, const System::UnicodeString St, double RotDegree) {
	if (RoundTo(RotDegree, 0) == 0.0f) {
		TextOut(x, y, St);
	}
	else {

		const int iWasX = x;
		const int iWasY = y;

		const double tmpTextWidth = TextWidth(St);
		const double tmpTextHeight = TextHeight(St);

		double dX = x;
		double dY = y;

		// �������� � Left, Top, � ����� ����� �������� � Bottom
		// ��� ��� � SVG, ����� �������� ���� ������������ ������ ����,
		// � ��� ������ ���� x=Left, y=Bottom
		if (TextAlign & TA_CENTER) {
			dX -= tmpTextWidth / 2.0f;
			dY -= tmpTextHeight / 2.0f;
		}

#if 0 // ��� ����� �������� �������
		UnicodeString sOut = L"";
		if (TextAlign == TA_LEFT) {
			sOut += L"TA_LEFT|TA_TOP|";
		}
		if (TextAlign & TA_RIGHT) {
			sOut += L"TA_RIGHT|";
		}
		if (TextAlign & TA_CENTER) {
			sOut += L"TA_CENTER|";
		}
		if (TextAlign & TA_BOTTOM) {
			sOut += L"TA_BOTTOM|";
		}
		if (TextAlign & TA_BASELINE) {
			sOut += L"TA_BASELINE|";
		}

		WLOG_DEBUG(L"TextAlign=[%s]", sOut.c_str());
#endif

		// � SVG ��������� � ���� �� ���������,
		// ������� ���� � ��� ������, �� ������
		if (!(TextAlign & TA_BOTTOM)) {
			dY += tmpTextHeight;
		}

		UnicodeString tmpSt = UnicodeString().sprintf(L"<text x=\"%.f\" y=\"%.f\"", dX, dY);

		const UnicodeString sFontHeight = this->UseTextPoints ? //
		UnicodeString().sprintf(L"%dpt", Font->Size) : //
		FormatFloat(L"0.##", (double)Font->Size * (double)Font->PixelsPerInch / 72.0f);

		tmpSt = tmpSt + L" font-family=" + AnsiQuotedStr(Font->Name, L'"') + " font-size=" + AnsiQuotedStr(sFontHeight, L'"');

		// x,y - �������� ������ ��������
		tmpSt = tmpSt + " transform=\"rotate(" + FormatFloat("0.##", -RotDegree) //
		+ " " + FormatFloat("0.##", iWasX) + " " + FormatFloat("0.##", iWasY) + ")\"";

		if (Font->Style.Contains(fsItalic)) {
			tmpSt = tmpSt + " font-style=\"italic\"";
		}

		if (Font->Style.Contains(fsBold)) {
			tmpSt = tmpSt + " font-weight=\"bold\"";
		}

		if (Font->Style.Contains(fsUnderline)) {
			tmpSt = tmpSt + " font-decoration=\"underline\"";
		}

		if (Font->Style.Contains(fsStrikeOut)) {
			tmpSt = tmpSt + " font-decoration=\"line-through\"";
		}

		tmpSt = tmpSt + " fill=" + SVGColor(Font->Color) + ">";

		Add(tmpSt);
		Add(VerifySpecial(St));
		Add("</text>");

#if 0 // DEBUG: uncomment if you want to see rotation point
		Add(UnicodeString().sprintf(L"<circle cx=\"%d\" cy=\"%d\" r=\"3\"  fill=\"red\" />", iWasX, iWasY));
#endif

	}
}

//---------------------------------------------------------------------------
void __fastcall TTreeSVGCanvasEx::StartGroup(const UnicodeString &sID, const UnicodeString &sClass) {
	UnicodeString sGroup = L"<g";
	if (!sID.IsEmpty()) {
		sGroup = sGroup + " id=\"" + VerifySpecial(sID) + "\"";
	}
	if (!sClass.IsEmpty()) {
		sGroup = sGroup + " class=\"" + VerifySpecial(sClass) + "\"";
	}
	sGroup = sGroup + " >";

	Add(sGroup);
}

//---------------------------------------------------------------------------
void __fastcall TTreeSVGCanvasEx::EndGroup(void) {
	Add("</g>");
}

//---------------------------------------------------------------------------
void __fastcall TTreeSVGCanvasEx::AddDesc(const UnicodeString &sDescription) {
	Add(L"<desc>" + VerifySpecial(sDescription) + L"</desc>");
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void __fastcall SaveTreeToSvg(TCustomTeePanel *APanel, const UnicodeString &sUnitName, const TRect &ARect, TStringList *AOutputList) {

	const int iWasHorizOffset = APanel->View3DOptions->HorizOffset;
	const int iWasVertOffset = APanel->View3DOptions->VertOffset;
	const int iWasZoom = APanel->View3DOptions->Zoom;
	const bool bWasAutoRepaint = APanel->AutoRepaint;

	try {
		TCanvas3D *AWasCanvas3D = APanel->Canvas;

		APanel->View3DOptions->Zoom = 100;
		APanel->View3DOptions->HorizOffset = -ARect.left;
		APanel->View3DOptions->VertOffset = -ARect.top;

		const TRect ASvgRect(0, 0, ARect.Width(), ARect.Height());

		APanel->AutoRepaint = false;

		// Protected across assemblies
		TTeePanelAccess * ATeePanelAccess = reinterpret_cast<TTeePanelAccess*>(APanel);
		ATeePanelAccess->InternalCanvas3D = NULL;

		try {
			/* DO NOT USE SMART POINTER !!! */
			TTreeSVGCanvasEx*ACanvas3D = new TTreeSVGCanvasEx(sUnitName, AOutputList);
			ACanvas3D->UseTextPoints = SettingsData->ExportSvgSettings->ExportSvgTextFontUsePoints;
			APanel->Canvas = ACanvas3D;
			APanel->Canvas->Assign(AWasCanvas3D);

			if (!APanel->Parent)
				APanel->BufferedDisplay = true;

			APanel->Draw(APanel->Canvas->ReferenceCanvas, ASvgRect);
		}
		__finally {
			APanel->Canvas = AWasCanvas3D;
		}
	}
	__finally {
		APanel->AutoRepaint = bWasAutoRepaint;

		APanel->View3DOptions->Zoom = iWasZoom;
		APanel->View3DOptions->HorizOffset = iWasHorizOffset;
		APanel->View3DOptions->VertOffset = iWasVertOffset;
	}
}
