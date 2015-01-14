#include <windows.h>
#include "Gwen/Renderers/DirectX10.h"
#include "Gwen/Utility.h"
#include "Gwen/Font.h"
#include "Gwen/Texture.h"
#include <D3d9types.h>
#include <math.h>
#include <vector>
#include <fstream>

#define SAFE_RELEASE(x) {if(x) x->Release(); x=0; }
#define RED   0x1
#define GREEN 0x2
#define BLUE  0x4
#define ALPHA 0x8
#define ALL (RED | GREEN | BLUE | ALPHA)

struct FontData
{
	ID3DX10Font*	pFont;
	int			iSpaceWidth;
};

const std::string GwenUIShader = std::string(

"SamplerState LinearFilter {"
"	AddressU = CLAMP;"
"	AddressV = CLAMP;"
"	AddressW = CLAMP;"
"	Filter = MIN_MAG_MIP_LINEAR;"
"};"
"struct PSstruct {"
"	float4 position : SV_Position;"
"	float4 color    : COLOR;    "
"	float2 texcoord : TexCoord;"
"};"
"Texture2D UITexture;"
"PSstruct VS(float2 position : POSITION, float4 color: COLOR, float2 texcoord: TEXCOORD) { "
	"PSstruct Out;" 
	"Out.position = float4(position.xy, 1, 1);"
	"Out.color = color;"
    "Out.texcoord = texcoord;" 
    "return Out;" 
"}"
"float4 PSTex( PSstruct In ) : SV_Target0 {"
"    return UITexture.Sample( LinearFilter, In.texcoord );" 
"}" 
"float4 PSCol( PSstruct In ) : SV_Target0 {"
"    return In.color;" 
"}" 
" " 
"technique10 UITechniqueCol {" 
"    pass P0 {" 
"       SetVertexShader( CompileShader( vs_4_0, VS() ) );" 
"        SetGeometryShader( NULL );"
"        SetPixelShader( CompileShader( ps_4_0, PSCol() ) );" 
"    } "
"}"
"technique10 UITechniqueTex{" 
"    pass P0 {" 
"       SetVertexShader( CompileShader( vs_4_0, VS() ) );" 
"        SetGeometryShader( NULL );"
"        SetPixelShader( CompileShader( ps_4_0, PSTex() ) );" 
"    } "
"}"

);


namespace Gwen
{
	namespace Renderer
	{
		DirectX10::DirectX10( ID3D10Device* pDevice )
		{
			m_pDevice = pDevice;
			m_iVertNum = 0;
			m_pInputLayout=0;
			m_pUIShader=0;
			m_pUITechniqueCol=m_pUITechniqueTex=ActiveTechnique=0;
			m_pUITexture=0;
			m_pUIVertexBuffer=0;
			m_pUIBlendState=0;

			m_pUIRasterizerStateNormal=m_pUIRasterizerStateScissor=0;

			HR(D3DX10CreateEffectFromMemory( GwenUIShader.c_str(), GwenUIShader.size()*sizeof(char), NULL, 0, 0, "fx_4_0", 0, 0, m_pDevice, NULL, NULL, &m_pUIShader, NULL, NULL ));
	
			m_pUITechniqueCol = m_pUIShader->GetTechniqueByName("UITechniqueCol");
			m_pUITechniqueTex = m_pUIShader->GetTechniqueByName("UITechniqueTex");
			ActiveTechnique = m_pUITechniqueTex;
			m_pUITexture  = m_pUIShader->GetVariableByName("UITexture")->AsShaderResource();

			const D3D10_INPUT_ELEMENT_DESC layout[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0,		0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0,		8, D3D10_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,       12, D3D10_INPUT_PER_VERTEX_DATA, 0 }
			};
			int numElements = sizeof( layout ) / sizeof( layout[0] );
			D3D10_PASS_DESC PassDesc;
			m_pUITechniqueCol->GetPassByIndex( 0 )->GetDesc( &PassDesc );
			HR( m_pDevice->CreateInputLayout( layout, numElements, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &m_pInputLayout ) );

			
			//create an empty, dynamic vertex buffer
			D3D10_BUFFER_DESC vbdesc;
			vbdesc.ByteWidth = MaxVerts * sizeof( VERTEXFORMAT2D );
			vbdesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
			vbdesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
			vbdesc.Usage = D3D10_USAGE_DYNAMIC;
			vbdesc.MiscFlags =0;
			HR(m_pDevice->CreateBuffer( &vbdesc, 0, &m_pUIVertexBuffer));
		

			// create a rast state to cull none and fill solid. Also create the scissor rast state
			D3D10_RASTERIZER_DESC descras;
			descras.CullMode = (D3D10_CULL_MODE) D3D10_CULL_NONE;
			descras.FillMode = (D3D10_FILL_MODE) D3D10_FILL_SOLID;
			descras.FrontCounterClockwise = FALSE;
			descras.DepthBias = 0;
			descras.DepthBiasClamp = 0.0f;
			descras.SlopeScaledDepthBias = 0.0f;
			descras.AntialiasedLineEnable = FALSE;
			descras.DepthClipEnable = FALSE;
			descras.MultisampleEnable = (BOOL) false;
			descras.ScissorEnable = (BOOL) false;
			HR(m_pDevice->CreateRasterizerState(&descras, &m_pUIRasterizerStateNormal));
			descras.ScissorEnable = true;
			HR(m_pDevice->CreateRasterizerState(&descras, &m_pUIRasterizerStateScissor));

			
			
			D3D10_BLEND_DESC blenddesc;
			blenddesc.AlphaToCoverageEnable = (BOOL) false;
			blenddesc.BlendOp = (D3D10_BLEND_OP) D3D10_BLEND_OP_ADD;
			blenddesc.SrcBlend = (D3D10_BLEND) D3D10_BLEND_SRC_ALPHA;
			blenddesc.DestBlend = (D3D10_BLEND) D3D10_BLEND_INV_SRC_ALPHA;
			blenddesc.BlendOpAlpha = (D3D10_BLEND_OP) D3D10_BLEND_OP_ADD;
			blenddesc.SrcBlendAlpha = (D3D10_BLEND) D3D10_BLEND_SRC_ALPHA;
			blenddesc.DestBlendAlpha = (D3D10_BLEND) D3D10_BLEND_INV_SRC_ALPHA;
			memset(&blenddesc.BlendEnable, 0, sizeof(blenddesc.BlendEnable));
			memset(&blenddesc.RenderTargetWriteMask, 0, sizeof(blenddesc.RenderTargetWriteMask));
			blenddesc.BlendEnable[0] = true;
			blenddesc.RenderTargetWriteMask[0] = ALL;
			HR(m_pDevice->CreateBlendState(&blenddesc, &m_pUIBlendState));

			D3D10_DEPTH_STENCIL_DESC depthdesc;
			memset(&depthdesc, 0, sizeof(depthdesc));
			depthdesc.DepthEnable = (BOOL) false;
			depthdesc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ZERO;
			depthdesc.DepthFunc = D3D10_COMPARISON_NEVER;
			depthdesc.StencilEnable = (BOOL) false;
			
			HR(m_pDevice->CreateDepthStencilState(&depthdesc, &m_pUIDepthState));

			DXGI_SAMPLE_DESC sample_desc = {
				1, 0
			};
			// create staging texture
			D3D10_TEXTURE2D_DESC staged_desc = {
				4096,//UINT Width;
				4096,//UINT Height;
				1,//UINT MipLevels;
				1,//UINT ArraySize;
				DXGI_FORMAT_R8G8B8A8_UINT,//DXGI_FORMAT Format;
				sample_desc,//DXGI_SAMPLE_DESC SampleDesc;
				D3D10_USAGE_STAGING,//D3D11_USAGE Usage;
				0,//UINT BindFlags;
				D3D10_CPU_ACCESS_READ,//UINT CPUAccessFlags;
				0//UINT MiscFlags;
			};
			m_pDevice->CreateTexture2D(&staged_desc, NULL, &m_StagingTexture);
		}

		DirectX10::~DirectX10()
		{
			SAFE_RELEASE(m_pInputLayout);
			SAFE_RELEASE(m_pUIShader);
			SAFE_RELEASE(m_pUIVertexBuffer);
			SAFE_RELEASE(m_pUIBlendState);
			SAFE_RELEASE(m_pUIDepthState);
			SAFE_RELEASE(m_pUIRasterizerStateScissor);
			SAFE_RELEASE(m_pUIRasterizerStateNormal);
			SAFE_RELEASE(m_StagingTexture);

		}

		void DirectX10::Begin()// get the states that this will change to set at the end call
		{
			ActiveTechnique=0;// set this to zero for this pass
			m_pDevice->OMGetBlendState(&m_pUILastBlendState, m_LastBlendFactor, &m_LastBlendMask);
			m_pDevice->RSGetState(&m_pUILastRasterizerState);
			m_pDevice->OMGetDepthStencilState(&m_LastDepthState, &m_LastStencilRef);
			m_pDevice->IAGetInputLayout(&m_LastInputLayout);
			m_pDevice->IAGetPrimitiveTopology(&m_LastTopology);
			m_pDevice->IAGetVertexBuffers(0, 8, m_LastBuffers, m_LastStrides, m_LastOffsets);

			m_pDevice->PSGetShader(&m_LastPSShader);
			m_pDevice->GSGetShader(&m_LastGSShader);
			m_pDevice->VSGetShader(&m_LastVSShader);

			float factor[4] = {0, 0, 0, 0};
			m_pDevice->OMSetBlendState(m_pUIBlendState, factor, ~0);
			m_pDevice->OMSetDepthStencilState(m_pUIDepthState, 0);
			m_pDevice->RSSetState( m_pUIRasterizerStateNormal);

		}

		void DirectX10::End()// reset the device to its original state
		{
			Flush();
			m_pDevice->OMSetBlendState(m_pUILastBlendState, m_LastBlendFactor, m_LastBlendMask);
			m_pDevice->RSSetState(m_pUILastRasterizerState);
			m_pDevice->OMSetDepthStencilState(m_LastDepthState, m_LastStencilRef);
			m_pDevice->IASetInputLayout(m_LastInputLayout);
			m_pDevice->IASetPrimitiveTopology(m_LastTopology);
			m_pDevice->IASetVertexBuffers(0, 8, m_LastBuffers, m_LastStrides, m_LastOffsets);
			m_pDevice->PSSetShader(m_LastPSShader);
			m_pDevice->GSSetShader(m_LastGSShader);
			m_pDevice->VSSetShader(m_LastVSShader);
		}

		void DirectX10::DrawLine( int x, int y, int a, int b )
		{
			Translate( x, y );
			Translate( a, b );	
		}

		void DirectX10::Flush()
		{
			if ( m_iVertNum > 0 )
			{
	
				//update the vertex buffer
				void* t = NULL;
				HR(m_pUIVertexBuffer->Map( D3D10_MAP_WRITE_DISCARD, 0, ( void** )&t ));
				memcpy( t, m_pVerts , m_iVertNum * sizeof( VERTEXFORMAT2D ) );
				m_pUIVertexBuffer->Unmap();
	
				// set topology to triangle strip
				m_pDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				m_pDevice->IASetInputLayout(m_pInputLayout);
				UINT stride [] = { sizeof(VERTEXFORMAT2D) };
				UINT offset [] = { 0 };
				// set the vertex buffer
				m_pDevice->IASetVertexBuffers(0, 1, &m_pUIVertexBuffer, stride, offset);
				// apply the correct technique 
				ActiveTechnique->GetPassByIndex( 0 )->Apply(0);
				//draw
				m_pDevice->Draw(m_iVertNum, 0);
				m_iVertNum = 0;
				
			}
		}

		void DirectX10::AddVert( int x, int y )
		{
			if ( m_iVertNum >= MaxVerts-1 )
			{
				Flush();
			}

			m_pVerts[ m_iVertNum ].x = (2.0f*(float)x/m_Wndx) -1.0f;
			m_pVerts[ m_iVertNum ].y = (-2.0f*(float)y/m_Wndy) +1.0f;
			m_pVerts[ m_iVertNum ].color = m_Color;

			m_iVertNum++;
		}

		void DirectX10::AddVert( int x, int y, float u, float v )
		{
			if ( m_iVertNum >= MaxVerts-1 )
			{
				Flush();
			}
			
			m_pVerts[ m_iVertNum ].x = (2.0f*(float)x/m_Wndx) -1.0f;
			m_pVerts[ m_iVertNum ].y = (-2.0f*(float)y/m_Wndy) +1.0f;
			m_pVerts[ m_iVertNum ].u = u;
			m_pVerts[ m_iVertNum ].v = v;
			
			m_pVerts[ m_iVertNum ].color = m_Color;

			m_iVertNum++;
		}

		void DirectX10::DrawFilledRect( Gwen::Rect rect )
		{
			if ( ActiveTechnique != m_pUITechniqueCol ){
				Flush();
				ActiveTechnique = m_pUITechniqueCol;

			}	
			
			Translate( rect );

			AddVert( rect.x, rect.y );
			AddVert( rect.x+rect.w, rect.y );
			AddVert( rect.x, rect.y + rect.h );

			AddVert( rect.x+rect.w, rect.y );
			AddVert( rect.x+rect.w, rect.y+rect.h );
			AddVert( rect.x, rect.y + rect.h );
		}

		void DirectX10::SetDrawColor(Gwen::Color color)
		{
			m_Color = D3DCOLOR_ARGB( color.a, color.r, color.g, color.b );
		}

		void DirectX10::LoadFont( Gwen::Font* font )
		{
			m_FontList.push_back( font );
			// Scale the font according to canvas
			font->realsize = font->size * Scale();
			
			ID3DX10Font* fontHndl;
			HR(D3DX10CreateFont(m_pDevice, font->realsize, 0, 0, 1, false, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, 
				font->realsize < 14 ? DEFAULT_QUALITY : CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, font->facename.c_str(), &fontHndl));
	

			FontData*	pFontData = new FontData();
			memset(pFontData, 0, sizeof(FontData));
			pFontData->pFont = fontHndl;

			// ID3DXFont doesn't measure trailing spaces, so we measure the width of a space here and store it
			// in the font data - then we can add it to the width when we measure text with trailing spaces.
			{
				RECT rctA = {0,0,0,0};
				pFontData->pFont->DrawTextW( 0, L"A", -1, &rctA, DT_CALCRECT | DT_LEFT | DT_TOP | DT_SINGLELINE, D3DXCOLOR(0.0f,0.0f,0.0f,0.0f) );

				RECT rctSpc = {0,0,0,0};
				pFontData->pFont->DrawTextW( 0, L"A A", -1, &rctSpc, DT_CALCRECT | DT_LEFT | DT_TOP | DT_SINGLELINE, D3DXCOLOR(0.0f,0.0f,0.0f,0.0f) );

				pFontData->iSpaceWidth = rctSpc.right - rctA.right * 2;
			}

			font->data = pFontData;
		}

		void DirectX10::FreeFont( Gwen::Font* pFont )
		{
			m_FontList.remove( pFont );

			if ( !pFont->data ) return;

			FontData* pFontData = (FontData*) pFont->data;

			if ( pFontData->pFont )
			{
				SAFE_RELEASE(pFontData->pFont);
			}

			delete pFontData;
			pFont->data = NULL;

		}

		void DirectX10::RenderText( Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString& text )
		{
			Flush();

			// If the font doesn't exist, or the font size should be changed
			if ( !pFont->data || fabs( pFont->realsize - pFont->size * Scale() ) > 2 )
			{
				FreeFont( pFont );
				LoadFont( pFont );
			}

			FontData* pFontData = (FontData*) pFont->data;
			
			Translate( pos.x, pos.y );

			RECT ClipRect = { pos.x, pos.y, 0, 0 };
			pFontData->pFont->DrawTextW( 0, text.c_str(), -1, &ClipRect, DT_LEFT | DT_TOP | DT_NOCLIP | DT_SINGLELINE, m_Color );
		
		}

		Gwen::Point DirectX10::MeasureText( Gwen::Font* pFont, const Gwen::UnicodeString& text )
		{
			// If the font doesn't exist, or the font size should be changed
			if ( !pFont->data || fabs( pFont->realsize - pFont->size * Scale() ) > 2 )
			{
				FreeFont( pFont );
				LoadFont( pFont );
			}

			FontData* pFontData = (FontData*) pFont->data;

			Gwen::Point size;

			if ( text.empty() )
			{
				RECT rct = {0,0,0,0};
				pFontData->pFont->DrawTextW( NULL, L"W", -1, &rct, DT_CALCRECT, D3DXCOLOR(0.0f,0.0f,0.0f,0.0f) );

				return Gwen::Point( 0, rct.bottom );
			}

			RECT rct = {0,0,0,0};
			pFontData->pFont->DrawTextW( NULL, text.c_str(), -1, &rct, DT_CALCRECT | DT_LEFT | DT_TOP | DT_SINGLELINE,  D3DXCOLOR(0.0f,0.0f,0.0f,0.0f));

			for (int i=text.length()-1; i>=0 && text[i] == L' '; i-- )
			{
				rct.right += pFontData->iSpaceWidth;
			}

			return Gwen::Point( rct.right / Scale(), rct.bottom / Scale() );

		}

		void DirectX10::StartClip()
		{
			Flush();

			m_pDevice->RSSetState( m_pUIRasterizerStateScissor );

			const Gwen::Rect& rect = ClipRegion();

			D3D10_RECT r;

			r.left = ceil( ((float)rect.x) * Scale() );
			r.right = ceil(((float)(rect.x + rect.w)) * Scale()) + 1;
			r.top = ceil( (float)rect.y * Scale() );
			r.bottom = ceil( ((float)(rect.y + rect.h)) * Scale() ) + 1;
		
			m_pDevice->RSSetScissorRects(1, &r );
		}

		void DirectX10::EndClip()
		{
			Flush();
			m_pDevice->RSSetState(m_pUIRasterizerStateNormal);
		}

		void DirectX10::DrawTexturedRect( Gwen::Texture* pTexture, Gwen::Rect rect, float u1, float v1, float u2, float v2 )
		{

			ID3D10ShaderResourceView* pImage = (ID3D10ShaderResourceView*) pTexture->data;

			// Missing image, not loaded properly?
			if ( !pImage )
			{
				return DrawMissingImage( rect );
			}

			Translate( rect );

			if ( ActiveTechnique != m_pUITechniqueTex )
			{
				Flush();
				ActiveTechnique = m_pUITechniqueTex;
			}	
			if( m_pCurrentTexture != pImage)
			{
				m_pUITexture->SetResource(pImage);
				m_pCurrentTexture =pImage;
			}
			AddVert( rect.x, rect.y,			u1, v1 );
			AddVert( rect.x+rect.w, rect.y,		u2, v1 );
			AddVert( rect.x, rect.y + rect.h,	u1, v2 );

			AddVert( rect.x+rect.w, rect.y,		u2, v1 );
			AddVert( rect.x+rect.w, rect.y+rect.h, u2, v2 );
			AddVert( rect.x, rect.y + rect.h, u1, v2 );
			
		}


		Gwen::Color DirectX10::PixelColour( Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color& col_default )
		{

			ID3D10ShaderResourceView* pImage = (ID3D10ShaderResourceView*) pTexture->data;
			if ( !pImage ) return col_default;

			ID3D10Texture2D *t;
			pImage->GetResource(reinterpret_cast<ID3D10Resource**>(&t));
			// this is how it must be done on DX10 and DX11. Textures cannot be used as a shader resource and have CPU read access.
			// the reason is to reduce dependencies for optimization reasons.
			//m_pDevice->CopyResource(m_StagingTexture, t);
			m_pDevice->CopySubresourceRegion(m_StagingTexture, 0, 0, 0, 0, t, 0, NULL);

			D3D10_MAPPED_TEXTURE2D te;
			HRESULT hr = (m_StagingTexture->Map(0, D3D10_MAP_READ, 0,  &te));
			
			DWORD* pixels = (DWORD*)te.pData;
			D3DXCOLOR color = pixels[te.RowPitch / sizeof(DWORD) * y + x];
			m_StagingTexture->Unmap(0);
			t->Release();
		
			return Gwen::Color( color.r*255, color.g*255, color.b*255, color.a*255 );
		}
		void DirectX10::LoadTexture( Gwen::Texture* pTexture )
		{	
			std::ifstream file(pTexture->name.GetUnicode().c_str());
			if(!file) return;// missingfile, dont attempt to load
			file.close();
			// get the info from the file 
			D3DX10_IMAGE_INFO finfo;
			memset(&finfo, 0, sizeof(D3DX10_IMAGE_INFO));
			HR(D3DX10GetImageInfoFromFile(pTexture->name.GetUnicode().c_str(), 0, &finfo, 0));
		
			ID3D10ShaderResourceView *resource(0);

			HR(D3DX10CreateShaderResourceViewFromFile( m_pDevice, pTexture->name.GetUnicode().c_str(), 0, 0, &resource, 0));
			pTexture->data = resource;// store this instead of the ID3D10Texture2D because a ID3D10Texture2D object can be retrived from ID3D10ShaderResourceView by calling GetResource
			pTexture->width = finfo.Width;
			pTexture->height = finfo.Height;

		}

		void DirectX10::FreeTexture( Gwen::Texture* pTexture )
		{

			ID3D10ShaderResourceView* pImage = (ID3D10ShaderResourceView*) pTexture->data;
			if ( !pImage ) return;

			pImage->Release();
			pTexture->data = NULL;
			return;
		}

		void DirectX10::Release()
		{
			Font::List::iterator it = m_FontList.begin();

			while ( it != m_FontList.end() )
			{
				FreeFont( *it );
				it = m_FontList.begin();
			}
		}
	}
}