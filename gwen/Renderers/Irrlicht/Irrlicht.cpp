
#include "Gwen/Renderers/Irrlicht.h"
#include "Gwen/Texture.h"

#pragma comment(lib, "Irrlicht.lib")

#include <iostream>

namespace Gwen
{
	namespace Renderer
	{
		//
		//	Irrlicht ICacheToTexture
		class IrrlichtCTT : public Gwen::Renderer::ICacheToTexture
		{
			irr::video::IVideoDriver* m_Driver;
			std::map<Gwen::Controls::Base*, irr::video::ITexture*> m_TextureCache;

		public:
			IrrlichtCTT(irr::video::IVideoDriver* Driver) :
			m_Driver(Driver) {}
			~IrrlichtCTT()
			{
				//
				//	Cleanup all cached textures
				for (auto it : m_TextureCache)
				{
					m_Driver->removeTexture(it.second);
				}
			}

			void Initialize() {}
			void ShutDown() {}

			//
			//	Setup Irrlicht to cache this control to a texture
			void SetupCacheTexture(Gwen::Controls::Base* control)
			{
				std::map<Gwen::Controls::Base*, irr::video::ITexture*>::iterator it = m_TextureCache.find(control);
				if (it != m_TextureCache.end())
				{
					m_Driver->setRenderTarget((*it).second, true, true, irr::video::SColor(0,0,0,0));
				}
			}

			//
			//	Revert Irrlicht's render target
			void FinishCacheTexture(Gwen::Controls::Base* control)
			{
				m_Driver->setRenderTarget(NULL, false, false);
			}

			//
			//	Draw this controls cached texture
			void DrawCachedControlTexture(Gwen::Controls::Base* control)
			{
				std::map<Gwen::Controls::Base*, irr::video::ITexture*>::iterator it = m_TextureCache.find(control);
				if (it != m_TextureCache.end())
				{
					m_Driver->draw2DImage((*it).second,
						irr::core::vector2di(control->X(), control->Y()),
						irr::core::rect<irr::s32>(irr::core::dimension2d<irr::s32>(0, 0),
						(*it).second->getSize()),
						NULL,
						irr::video::SColor(255, 255, 255, 255),
						true);
				}
			}

			//
			//	Create this controls cached texture if it doesn't already exist
			void CreateControlCacheTexture(Gwen::Controls::Base* control)
			{
				if (m_TextureCache.find(control) == m_TextureCache.end())
				{
					const Gwen::Rect Bounds = control->GetBounds();
					irr::video::ITexture* RTT = m_Driver->addRenderTargetTexture(irr::core::dimension2d<irr::u32>(Bounds.w, Bounds.h));
					m_TextureCache.insert(std::pair<Gwen::Controls::Base*, irr::video::ITexture*>(control, RTT));
				}
			}

			void UpdateControlCacheTexture(Gwen::Controls::Base* control) {}

			void SetRenderer(Gwen::Renderer::Base* renderer) {}
		};

		//
		//	Irrlicht Renderer
		Irrlicht::Irrlicht(irr::IrrlichtDevice* Device) :
			m_CTT(new IrrlichtCTT(Device->getVideoDriver())), Driver(Device->getVideoDriver())
		{
			DrawColor.set(255, 255, 255, 255);
			ClipRect = irr::core::rect<irr::s32>();
			Text = Device->getGUIEnvironment()->getBuiltInFont();
		}

		Irrlicht::~Irrlicht()
		{
			delete m_CTT;
		}

		/*void Irrlicht::Init() {}

		void Irrlicht::Begin() {}

		void Irrlicht::End() {}*/

		void Irrlicht::SetDrawColor(Gwen::Color color)
		{
			DrawColor.set((irr::u32)color.a, (irr::u32)color.r, (irr::u32)color.g, (irr::u32)color.b);
		}

		void Irrlicht::DrawFilledRect(Gwen::Rect rect)
		{
			Translate(rect);
			Driver->draw2DRectangle(DrawColor,
				irr::core::rect<irr::s32>(rect.x, rect.y, rect.x + rect.w, rect.y + rect.h),
				&ClipRect);
		}

		void Irrlicht::StartClip()
		{
			const Gwen::Rect rect = ClipRegion();
			ClipRect = irr::core::rect<irr::s32>(rect.x*Scale(), rect.y*Scale(), (rect.x + rect.w)*Scale(), (rect.y + rect.h)*Scale());
		}

		void Irrlicht::EndClip()
		{
			ClipRect = irr::core::rect<irr::s32>();
		}

		void Irrlicht::LoadTexture(Gwen::Texture* pTexture)
		{
			irr::video::ITexture* NewTex = Driver->getTexture(pTexture->name.c_str());
			if (!NewTex) {
				pTexture->failed = true;
				return;
			}

			const irr::core::dimension2d<irr::u32> TexSize = NewTex->getSize();
			pTexture->width = TexSize.Width;
			pTexture->height = TexSize.Height;
			pTexture->data = NewTex;
		}

		void Irrlicht::FreeTexture(Gwen::Texture* pTexture)
		{
			if (!pTexture->FailedToLoad())
			{
				Driver->removeTexture((irr::video::ITexture*)pTexture->data);
			}
		}

		void Irrlicht::DrawTexturedRect(Gwen::Texture* pTexture, Gwen::Rect pTargetRect, float u1, float v1, float u2, float v2)
		{
			if (!pTexture->FailedToLoad())
			{
				const unsigned int w = pTexture->width;
				const unsigned int h = pTexture->height;

				Translate(pTargetRect);
				
				Driver->draw2DImage((irr::video::ITexture*)pTexture->data,
					irr::core::rect<irr::s32>(pTargetRect.x, pTargetRect.y, pTargetRect.x + pTargetRect.w, pTargetRect.y + pTargetRect.h),
					irr::core::rect<irr::s32>(u1*w, v1*h, u2*w, v2*h),
					&ClipRect,
					0,
					true);
			}
		}

		//void Irrlicht::DrawMissingImage(Gwen::Rect pTargetRect) {}

		Gwen::Color Irrlicht::PixelColour(Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color & col_default)
		{
			if (pTexture->FailedToLoad())
			{
				return col_default;
			}

			irr::video::ITexture* Texture = (irr::video::ITexture*)pTexture->data;

			irr::video::SColor PixelColor;
			const irr::u32 pitch = Texture->getPitch();
			const irr::video::ECOLOR_FORMAT format = Texture->getColorFormat();
			static const irr::u32 bytes = irr::video::IImage::getBitsPerPixelFromFormat(format) / 8;

			unsigned char* buffer = (unsigned char*)Texture->lock();
			if (buffer)
			{
				PixelColor = irr::video::SColor(*(unsigned int*)(buffer + (y * pitch) + (x*bytes)));
				Texture->unlock();
				return Gwen::Color(PixelColor.getRed(), PixelColor.getGreen(), PixelColor.getBlue(), PixelColor.getAlpha());
			}

			return col_default;
		}

		ICacheToTexture* Irrlicht::GetCTT()
		{
			return m_CTT;
		}

		void Irrlicht::LoadFont(Gwen::Font* pFont)
		{}

		void Irrlicht::FreeFont(Gwen::Font* pFont)
		{}

		void Irrlicht::RenderText(Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString & text)
		{
			Translate(pos.x, pos.y);
			Text->draw(text.c_str(),
				irr::core::rect<irr::s32>(pos.x, pos.y, pos.x, pos.y),
				DrawColor,
				false,
				false,
				&ClipRect);
		}

		/*Gwen::Point Irrlicht::MeasureText(Gwen::Font* pFont, const Gwen::UnicodeString & text)
		{
		Gwen::Point NewPoint;
		NewPoint.x = 3;
		NewPoint.y = 8;
		return NewPoint;
		}*/

		void Irrlicht::DrawLinedRect(Gwen::Rect rect)
		{
			Translate(rect);
			Driver->draw2DRectangleOutline(irr::core::rect<irr::s32>(rect.x, rect.y, rect.x + rect.w, rect.y + rect.h), DrawColor);
		}

		void Irrlicht::DrawPixel(int x, int y)
		{
			Driver->drawPixel(x, y, DrawColor);
		}

	}
}