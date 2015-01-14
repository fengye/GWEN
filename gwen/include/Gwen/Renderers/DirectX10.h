/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/

#ifndef GWEN_RENDERERS_DIRECTX10_H
#define GWEN_RENDERERS_DIRECTX10_H
#include "Gwen/Gwen.h"
#include "Gwen/BaseRender.h"
#include <d3d10.h>
#include <d3dx10.h>

#if defined(_DEBUG) | defined (DEBUG)
	#include <dxerr.h>
	#include <iostream>
	#pragma comment(lib, "dxerr") // needed for dxtrace
	#ifndef HR
	#define HR(x){																\
		HRESULT hr = (x);														\
		if(FAILED(hr)){															\
			std::cout<<"An error occured on line"<<(DWORD)__LINE__<<" in the file "<<__FILE__<<std::endl; \
			std::cout<<DXGetErrorStringA(hr)<<std::endl<<DXGetErrorDescriptionA(hr)<<std::endl;   \
		}																		\
	}
	#endif
	#ifndef OUTPUT_DEBUG_MSG // nothing special about this, I use the console for standard debug info
	#define OUTPUT_DEBUG_MSG(x) {												\
		std::cout<<x<<std::endl;													     	\
	}
	#endif
#else
	#ifndef HR
	#define HR(x) (x)
	#endif
	#ifndef OUTPUT_DEBUG_MSG
	#define OUTPUT_DEBUG_MSG(x) {}
	#endif
#endif 

struct VERTEXFORMAT2D
{
	FLOAT x, y;
	DWORD color;
	float u, v;
};


namespace Gwen 
{
	namespace Renderer 
	{

		class GWEN_EXPORT DirectX10 : public Gwen::Renderer::Base
		{
			public:

				DirectX10( ID3D10Device* pDevice );
				~DirectX10();

				virtual void Begin();
				virtual void End();
				virtual void Release();

				virtual void SetDrawColor(Gwen::Color color);

				virtual void DrawLine( int x, int y, int a, int b );
				virtual void DrawFilledRect( Gwen::Rect rect );

				virtual void LoadFont( Gwen::Font* pFont );
				virtual void FreeFont( Gwen::Font* pFont );
				virtual void RenderText( Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString& text );
				virtual Gwen::Point MeasureText( Gwen::Font* pFont, const Gwen::UnicodeString& text );
				virtual Gwen::Color PixelColour( Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color& col_default = Gwen::Color( 255, 255, 255, 255 ) );

				void StartClip();
				void EndClip();

				void DrawTexturedRect( Gwen::Texture* pTexture, Gwen::Rect pTargetRect, float u1=0.0f, float v1=0.0f, float u2=1.0f, float v2=1.0f );
				void LoadTexture( Gwen::Texture* pTexture );
				void FreeTexture( Gwen::Texture* pTexture );
				// set screensize should be called each time the screen size changes. This will allow the text to adjust itself to a change in the resolution.
				//Otherwise, the text will grow with the size of the window, and visa versa
				void SetScreenSize(float x, float y) { m_Wndx =x; m_Wndy =y; }
			protected:

				void*				m_pCurrentTexture;
				ID3D10Device*		m_pDevice;
				DWORD				m_Color;
				float				m_Wndx, m_Wndy;// screen size in pixels
				void Flush();
				void AddVert( int x, int y );
				void AddVert( int x, int y, float u, float v );

				static const int		MaxVerts = 1024;
				VERTEXFORMAT2D			m_pVerts[MaxVerts];
				int						m_iVertNum;
			
				ID3D10InputLayout*		m_pInputLayout;
				ID3D10Effect*			m_pUIShader;
				ID3D10EffectTechnique*	m_pUITechniqueCol, *m_pUITechniqueTex, *ActiveTechnique;
				ID3D10EffectShaderResourceVariable* m_pUITexture;
				ID3D10Buffer*			m_pUIVertexBuffer;
				ID3D10BlendState*		m_pUILastBlendState, *m_pUIBlendState;
				ID3D10DepthStencilState* m_pUIDepthState;
				float					m_LastBlendFactor[4];
				UINT					m_LastBlendMask, m_LastStencilRef;
				ID3D10InputLayout*		m_LastInputLayout;
				D3D10_PRIMITIVE_TOPOLOGY m_LastTopology;
				ID3D10Buffer*			m_LastBuffers[8];
				UINT					m_LastStrides[8], m_LastOffsets[8];
				ID3D10PixelShader*		m_LastPSShader;
				ID3D10VertexShader*		m_LastVSShader;
				ID3D10GeometryShader*	m_LastGSShader;
				ID3D10DepthStencilState* m_LastDepthState;
				ID3D10RasterizerState*	m_pUILastRasterizerState, *m_pUIRasterizerStateScissor, *m_pUIRasterizerStateNormal;
				ID3D10Texture2D*			m_StagingTexture;
				Gwen::Font::List		m_FontList;
	

		};

	}
}
#endif
