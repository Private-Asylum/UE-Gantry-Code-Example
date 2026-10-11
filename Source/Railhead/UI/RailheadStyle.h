// Copyright Private Asylum LLC, 2026, All Rights Reserved.

#pragma once

#include "Brushes/SlateColorBrush.h"
#include "Fonts/SlateFontInfo.h"
#include "Math/Color.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"

struct FSlateBrush;

/**
 * Railhead's look, in one place: brass on soot. Every Railhead widget takes its fonts and colours
 * from here, so restyling the game is one file.
 */
namespace RailheadStyle
{
	inline FSlateFontInfo Font(int32 Size, bool bBold = false)
	{
		return FCoreStyle::GetDefaultFontStyle(bBold ? "Bold" : "Regular", Size);
	}

	/** Plain filled box, tinted by whoever uses it. */
	inline const FSlateBrush* Fill()
	{
		return FCoreStyle::Get().GetBrush("GenericWhiteBox");
	}

	/** A gauge: a dark track and a plain fill the widget tints. The engine's default fill is a blue image. */
	inline const FProgressBarStyle& Gauge()
	{
		static const FProgressBarStyle Style = FProgressBarStyle()
			.SetBackgroundImage(FSlateColorBrush(FLinearColor(0.0f, 0.0f, 0.0f, 0.6f)))
			.SetFillImage(FSlateColorBrush(FLinearColor::White))
			.SetMarqueeImage(FSlateColorBrush(FLinearColor::White));
		return Style;
	}

	/** A menu button: soot, brass when hovered, darker when pressed. */
	inline const FButtonStyle& Button()
	{
		static const FButtonStyle Style = FButtonStyle()
			.SetNormal(FSlateColorBrush(FLinearColor(0.03f, 0.03f, 0.035f, 0.85f)))
			.SetHovered(FSlateColorBrush(FLinearColor(0.55f, 0.38f, 0.12f, 0.95f)))
			.SetPressed(FSlateColorBrush(FLinearColor(0.35f, 0.22f, 0.06f, 1.0f)))
			.SetDisabled(FSlateColorBrush(FLinearColor(0.03f, 0.03f, 0.035f, 0.4f)))
			.SetNormalPadding(FMargin(18.0f, 10.0f))
			.SetPressedPadding(FMargin(18.0f, 11.0f, 18.0f, 9.0f));
		return Style;
	}

	/** A text box on the menus: dark field, light text. */
	inline const FEditableTextBoxStyle& TextBox()
	{
		static const FEditableTextBoxStyle Style = FEditableTextBoxStyle(FCoreStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox"))
			.SetBackgroundImageNormal(FSlateColorBrush(FLinearColor(0.0f, 0.0f, 0.0f, 0.55f)))
			.SetBackgroundImageHovered(FSlateColorBrush(FLinearColor(0.02f, 0.02f, 0.02f, 0.7f)))
			.SetBackgroundImageFocused(FSlateColorBrush(FLinearColor(0.05f, 0.04f, 0.02f, 0.85f)))
			.SetForegroundColor(FSlateColor(FLinearColor(0.93f, 0.9f, 0.82f)))
			.SetFont(Font(15))
			.SetPadding(FMargin(10.0f, 7.0f));
		return Style;
	}

	inline const FLinearColor Brass(0.95f, 0.72f, 0.32f);
	inline const FLinearColor Paper(0.93f, 0.9f, 0.82f);
	inline const FLinearColor Dim(0.6f, 0.58f, 0.52f);
	inline const FLinearColor Soot(0.02f, 0.02f, 0.025f, 0.72f);
	inline const FLinearColor Black(0.0f, 0.0f, 0.0f, 1.0f);
	inline const FLinearColor Ember(1.0f, 0.38f, 0.08f);
}
