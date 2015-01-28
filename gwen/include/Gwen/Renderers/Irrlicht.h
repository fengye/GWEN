#pragma once

#ifndef GWEN_RENDERERS_IRRLICHT_H
#define GWEN_RENDERERS_IRRLICHT_H

#include "Gwen\Gwen.h"
#include "Gwen\BaseRender.h"

#include <Irrlicht.h>

namespace Gwen
{
	namespace Renderer
	{
		//
		//	Irrlicht ICacheToTexture
		class IrrlichtCTT;

		//
		//	Irrlicht Renderer
		class Irrlicht : public Gwen::Renderer::Base
		{
			IrrlichtCTT* m_CTT;
			irr::video::IVideoDriver* Driver;
			irr::video::SColor DrawColor;
			irr::core::rect<irr::s32> ClipRect;
			irr::gui::IGUIFont* Text;

		public:
			Irrlicht(irr::IrrlichtDevice* Device);
			~Irrlicht();

			/*void Init();
			void Begin();
			void End();*/

			void SetDrawColor(Gwen::Color color);

			void DrawFilledRect(Gwen::Rect rect);

			void StartClip();
			void EndClip();

			void LoadTexture(Gwen::Texture* pTexture);
			void FreeTexture(Gwen::Texture* pTexture);

			void DrawTexturedRect(Gwen::Texture* pTexture, Gwen::Rect pTargetRect, float u1 = 0.0f, float v1 = 0.0f, float u2 = 1.0f, float v2 = 1.0f);
			//void DrawMissingImage(Gwen::Rect pTargetRect);

			Gwen::Color PixelColour(Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color & col_default = Gwen::Color(255, 255, 255, 255));

			ICacheToTexture* GetCTT();

			void LoadFont(Gwen::Font* pFont);
			void FreeFont(Gwen::Font* pFont);
			void RenderText(Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString & text);
			//Gwen::Point MeasureText(Gwen::Font* pFont, const Gwen::UnicodeString & text);

			void DrawLinedRect(Gwen::Rect rect);
			void DrawPixel(int x, int y);

		};
	}
}
#endif