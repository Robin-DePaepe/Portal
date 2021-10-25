#include "stdafx.h"
#include "SpriteFontLoader.h"
#include "BinaryReader.h"
#include "ContentManager.h"
#include "TextureData.h"

SpriteFont* SpriteFontLoader::LoadContent(const std::wstring& assetFile)
{
	auto pBinReader = new BinaryReader();
	pBinReader->Open(assetFile);

	if (!pBinReader->Exists())
	{
		Logger::LogFormat(LogLevel::Error, L"SpriteFontLoader::LoadContent > Failed to read the assetFile!\nPath: \'%s\'", assetFile.c_str());
		return nullptr;
	}

	//See BMFont Documentation for Binary Layout

	unsigned int header{};
	char blockID{};
	int blockSize{};

	auto ReadNewBlockHeader = [&](const int blockNummer) mutable
	{
		pBinReader->SetBufferPosition(blockSize + header);
		header += blockSize;

		blockID = pBinReader->Read<char>();
		blockSize = pBinReader->Read<int>();
		header += 5;

		if (blockID != blockNummer)
		{
			Logger::LogFormat(LogLevel::Error, L"SpriteFontLoader::LoadContent > Error during parsing.\nPath: \'%s\'", assetFile.c_str());
		}
	};

	//Parse the Identification bytes (B,M,F)
	//If Identification bytes doesn't match B|M|F,
	//Log Error (SpriteFontLoader::LoadContent > Not a valid .fnt font) &

	if (pBinReader->Read<char>() != 'B' || pBinReader->Read<char>() != 'M' || pBinReader->Read<char>() != 'F')
	{
		Logger::LogFormat(LogLevel::Error, L"SpriteFontLoader::LoadContent > Not a valid .fnt font.\nPath: \'%s\'", assetFile.c_str());
		return nullptr;
	}
	header += 3;
	//Parse the version (version 3 required)
	//If version is < 3,
	//Log Error (SpriteFontLoader::LoadContent > Only .fnt version 3 is supported)

	char version{ pBinReader->Read<char>() };
	header += 1;

	if (version < 3)
	{
		Logger::LogFormat(LogLevel::Error, L"SpriteFontLoader::LoadContent >  Only .fnt version 3+ is supported.\nPath: \'%s\'", assetFile.c_str());
		return nullptr;
	}

	//Valid .fnt file
	auto pSpriteFont = new SpriteFont();
	//SpriteFontLoader is a friend class of SpriteFont
	//That means you have access to its privates (pSpriteFont->m_FontName = ... is valid)
	//**********
	// BLOCK 1
	//**********
	//Retrieve the blockId and blockSize
	ReadNewBlockHeader(1);

	//Retrieve the FontSize (will be -25, BMF bug) [SpriteFont::m_FontSize]
	pSpriteFont->m_FontSize = pBinReader->Read<short int>();
	//Move the binreader to the start of the FontName [BinaryReader::MoveBufferPosition(...) or you can set its position using BinaryReader::SetBufferPosition(...))
	pBinReader->SetBufferPosition(14 + header);//start of fontName + header )
	//Retrieve the FontName [SpriteFont::m_FontName]
	pSpriteFont->m_FontName = pBinReader->ReadNullString();


	//**********
	// BLOCK 2 *
	//**********
	ReadNewBlockHeader(2);

	//Retrieve Texture Width & Height [SpriteFont::m_TextureWidth/m_TextureHeight]
	pBinReader->SetBufferPosition(4 + header);//height location

	pSpriteFont->m_TextureHeight = pBinReader->Read<short int>();
	pSpriteFont->m_TextureWidth = pBinReader->Read<short int>();
	//Retrieve PageCount
	int pageCount{ pBinReader->Read<short int>() };
	//> if pagecount > 1
	if (pageCount > 1)
	{
		Logger::LogFormat(LogLevel::Error, L"SpriteFontLoader::LoadContent > Only one texture per font allowed.\nPath: \'%s\'", assetFile.c_str());
	}

	//**********
	// BLOCK 3 *
	//**********
	ReadNewBlockHeader(3);

	//Retrieve the PageName (store Local)
	std::wstring pageName{ pBinReader->ReadNullString() };

	//	> If PageName is empty
	if (pageName == L"")
	{
		Logger::LogFormat(LogLevel::Error, L"SpriteFontLoader::LoadContent > SpriteFont (.fnt): Invalid Font Sprite.\nPath: \'%s\'", assetFile.c_str());
	}
	//	> Log Error (SpriteFontLoader::LoadContent > SpriteFont (.fnt): Invalid Font Sprite [Empty])
	//>Retrieve texture filepath from the assetFile path
	const size_t size{ assetFile.find_last_of('/') };
	const std::wstring path{ assetFile.substr(0, size + 1) };
	//> (ex. c:/Example/somefont.fnt => c:/Example/) [Have a look at: wstring::rfind()]
	//>Use path and PageName to load the texture using the ContentManager [SpriteFont::m_pTexture]
	pSpriteFont->m_pTexture = ContentManager::Load<TextureData>(path + pageName);


	//**********
	// BLOCK 4 *
	//**********
	//Retrieve the blockId and blockSize
	ReadNewBlockHeader(4);

	//Retrieve Character Count (see documentation)
	int numChars{ blockSize / 20 }; //20 is size of the charinfo structure
	//Retrieve Every Character, For every Character:

	for (int i = 0; i < numChars; i++)
	{
		//> Retrieve CharacterId (store Local) and cast to a 'wchar_t'
		wchar_t charID{ wchar_t(pBinReader->Read<int>()) };
		//> Check if CharacterId is valid (SpriteFont::IsCharValid), Log Warning and advance to next character if not valid
		if (pSpriteFont->IsCharValid(charID))
		{
			//> Retrieve the corresponding FontMetric (SpriteFont::GetMetric) [REFERENCE!!!]
			FontMetric& metric{ pSpriteFont->GetMetric(charID) };
			//> Set IsValid to true [FontMetric::IsValid]
			metric.IsValid = true;
			//> Set Character (CharacterId) [FontMetric::Character]
			metric.Character = charID;
			//> Retrieve Xposition (store Local)
			short xPos{ pBinReader->Read<short int>() };
			//> Retrieve Yposition (store Local)
			short yPos{ pBinReader->Read<short int>() };
			//> Retrieve & Set Width [FontMetric::Width]
			metric.Width = pBinReader->Read<short int>();
			//> Retrieve & Set Height [FontMetric::Height]
			metric.Height = pBinReader->Read<short int>();
			//> Retrieve & Set OffsetX [FontMetric::OffsetX]
			metric.OffsetX = pBinReader->Read<short int>();
			//> Retrieve & Set OffsetY [FontMetric::OffsetY]
			metric.OffsetY = pBinReader->Read<short int>();
			//> Retrieve & Set AdvanceX [FontMetric::AdvanceX]
			metric.AdvanceX = pBinReader->Read<short int>();
			//> Retrieve & Set Page [FontMetric::Page]
			metric.Page = pBinReader->Read<char>();
			//> Retrieve Channel (BITFIELD!!!) 
			metric.Channel = pBinReader->Read<char>();
			//	> See documentation for BitField meaning [FontMetrix::Channel]

			//remap channel to BGRA (documentation)
			if (metric.Channel & 0b0001)
			{
				metric.Channel = 2;
			}
			else if (metric.Channel & 0b0010)
			{
				metric.Channel = 1;
			}
			else if (metric.Channel & 0b0100)
			{
				metric.Channel = 0;
			}
			else if (metric.Channel & 0b1000)
			{
				metric.Channel = 3;
			}
			//> Calculate Texture Coordinates using Xposition, Yposition, TextureWidth & TextureHeight [FontMetric::TexCoord]	
			metric.TexCoord.x = float(xPos) / pSpriteFont->m_TextureWidth;
			metric.TexCoord.y = float(yPos) / pSpriteFont->m_TextureHeight;
		}
		else
		{
			Logger::LogFormat(LogLevel::Warning, L"SpriteFontLoader::LoadContent >Invalid character found.\nPath: \'%s\'", assetFile.c_str());
			pBinReader->MoveBufferPosition(16); //move to start next char
		}
	}
	delete pBinReader;
	return pSpriteFont;
}

void SpriteFontLoader::Destroy(SpriteFont* objToDestroy)
{
	SafeDelete(objToDestroy);
}
