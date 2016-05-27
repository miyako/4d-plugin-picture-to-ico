/* --------------------------------------------------------------------------------
 #
 #	4DPlugin.cpp
 #	source generated by 4D Plugin Wizard
 #	Project : PICTURE TO ICO
 #	author : miyako
 #	2016/05/27
 #
 # --------------------------------------------------------------------------------*/


#include "4DPluginAPI.h"
#include "4DPlugin.h"

void PluginMain(PA_long32 selector, PA_PluginParameters params)
{
	try
	{
		PA_long32 pProcNum = selector;
		sLONG_PTR *pResult = (sLONG_PTR *)params->fResult;
		PackagePtr pParams = (PackagePtr)params->fParameters;

		CommandDispatcher(pProcNum, pResult, pParams); 
	}
	catch(...)
	{

	}
}

void CommandDispatcher (PA_long32 pProcNum, sLONG_PTR *pResult, PackagePtr pParams)
{
	switch(pProcNum)
	{
// --- PICTURE TO ICO

		case 1 :
			PICTURE_TO_ICO(pResult, pParams);
			break;

	}
}

#pragma mark -

// -------------------------------- PICTURE TO ICO --------------------------------

void getBMP(PA_Picture *picture, std::vector<uint8_t> &buf)
{
	size_t _w = 0;
	size_t _h = 0;
#if VERSIONMAC
	CGImageRef image = (CGImageRef)PA_CreateNativePictureForScreen(*picture);
#else
	Gdiplus::Bitmap *image = (Gdiplus::Bitmap *)PA_CreateNativePictureForScreen(*picture);
#endif
	buf.clear();
	if(image)
	{
#if VERSIONMAC
		
		_w = CGImageGetWidth(image);
		_h = CGImageGetHeight(image);
		
		size_t size = _w * _h;
		buf.resize(size);
		
		CGContextRef ctx = NULL;
		CGColorSpaceRef colorSpace = NULL;
		
		size_t bitmapBytesPerRow   = (_w * 4);
		size_t bitmapByteCount     = (bitmapBytesPerRow * _h);
		
		std::vector<uint8_t> bitmapData(bitmapByteCount);
		
		colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
		
		if (colorSpace)
		{
			ctx = CGBitmapContextCreate ((void *)&bitmapData[0],
																	 _w,
																	 _h,
																	 8,      // bits per component
																	 bitmapBytesPerRow,
																	 colorSpace,
																	 kCGImageAlphaPremultipliedFirst);
			
			CGColorSpaceRelease(colorSpace);
			
		}
		
		if (ctx)
		{
			CGRect rect = {{0,0},{_w,_h}};
			
			CGContextDrawImage(ctx, rect, image);
			
			size_t *pixels = (size_t *)CGBitmapContextGetData (ctx);
			
			size_t pixel, y8;
			size_t i = 0;
#if __LP64__
			BOOL alt = false;
			size_t j = 0;
#endif
			for(size_t y = 0; y < _h; y++)
			{
				for(size_t x = 0; x < _w; x++)
				{
#if __LP64__
					if(!alt)
					{
						pixel = pixels[j];
						y8 = (pixel >> 24) & 0xFF;
					}
					else
					{
						pixel = pixels[j];
						y8 = (pixel >> 56) & 0xFF;
						j++;
					}
					buf[i] = y8;
					i++;
					alt = !alt;
#else
					pixel = pixels[i];
					y8 = (pixel >> 24) & 0xFF;
					buf[i] = y8;
					i++;
#endif
				}
			}
			CGContextRelease(ctx);
		}
		
#else
		_w = image->GetWidth();
		_h = image->GetHeight();
		
		size_t size = _w * _h;
		std::vector<uint8_t> buf(size);
		
		uint32_t y8;
		size_t i = 0;
		
		for(size_t y = 0; y < _h; y++)
		{
			PA_YieldAbsolute();
			for(size_t x = 0; x < _w; x++)
			{
				Gdiplus::Color c;
				image->GetPixel(x,y,&c);
				y8 = c.GetR();
				buf[i] = y8;
				i++;
			}
		}
#endif
#if VERSIONMAC
		CGImageRelease(image); image = NULL;
#else
		delete image; image = NULL;
#endif
	}
}

void getPNG(PA_Picture *picture, std::vector<uint8_t> &buf)
{
	std::string type(".png");
	buf.clear();
	PA_ErrorCode err = eER_NoErr;
	unsigned i = 0;
	PA_Unistring t;
	std::map<CUTF8String, uint32_t> types;
	while (err == eER_NoErr)
	{
		t = PA_GetPictureData(*picture, ++i, NULL);
		err = PA_GetLastError();
		if(err == eER_NoErr)
		{
			uint32_t len = (uint32_t)(t.fLength * 4) + sizeof(uint8_t);
			std::vector<uint8_t> u(len);
			PA_ConvertCharsetToCharset(
																 (char *)t.fString,
																 t.fLength * sizeof(PA_Unichar),
																 eVTC_UTF_16,
																 (char *)&u[0],
																 len,
																 eVTC_UTF_8
																 );
			CUTF8String uti;
			uti = CUTF8String((const uint8_t *)&u[0]);
			CUTF8String typestring;
			size_t pos, found;
			found = 0;
			for(pos = uti.find(';'); pos != CUTF8String::npos; pos = uti.find(';', found))
			{
				typestring = uti.substr(found, pos-found);
				found = pos + 1;
				types.insert(std::map<CUTF8String, uint32_t>::value_type(typestring, i));
			}
			typestring = uti.substr(found, uti.length()-found);
			types.insert(std::map<CUTF8String, uint32_t>::value_type(typestring, i));
		}
	}
	std::map<CUTF8String, uint32_t>::iterator itr;
	itr = types.find((const uint8_t *)type.c_str());
	if (itr != types.end())
	{
		uint32_t pos = itr->second;
		PA_Handle h = PA_NewHandle(0);
		err = eER_NoErr;
		PA_GetPictureData(*picture, pos, h);
		err = PA_GetLastError();
		if(err == eER_NoErr)
		{
			unsigned long insize = PA_GetHandleSize(h);
			buf.resize(insize);
			memcpy(&buf[0], (const void *)PA_LockHandle(h), insize);
			PA_UnlockHandle(h);
			PA_DisposeHandle(h);
		}
	}
}

#pragma mark -

void getIcons(PA_Picture *picture,
										std::vector<uint8_t> &png16,
										std::vector<uint8_t> &png32,
										std::vector<uint8_t> &png48,
										std::vector<uint8_t> &png64,
										std::vector<uint8_t> &png128,
										std::vector<uint8_t> &png256)
{
	//96 is automatically rendered by windows
	PA_Picture icon16, icon32, icon48, icon64, icon128, icon256;
	
	PA_Variable args[4];
	
	args[0] = PA_CreateVariable(eVK_Picture);
	args[2] = PA_CreateVariable(eVK_Longint);
	args[3] = PA_CreateVariable(eVK_Longint);
	
	PA_SetPictureVariable(&args[0], PA_DuplicatePicture(*picture, 1));
	
	//Small
	args[1] = PA_CreateVariable(eVK_Picture);
	PA_SetLongintVariable(&args[2], 16);
	PA_SetLongintVariable(&args[3], 16);
	PA_ExecuteCommandByID(679, args, 4);
	icon16 = PA_DuplicatePicture(PA_GetPictureVariable(args[1]), 1);
	PA_ClearVariable(&args[1]);
	
	//
	args[1] = PA_CreateVariable(eVK_Picture);
	PA_SetLongintVariable(&args[2], 32);
	PA_SetLongintVariable(&args[3], 32);
	PA_ExecuteCommandByID(679, args, 4);
	icon32 = PA_DuplicatePicture(PA_GetPictureVariable(args[1]), 1);
	PA_ClearVariable(&args[1]);
	
	//Medium
	args[1] = PA_CreateVariable(eVK_Picture);
	PA_SetLongintVariable(&args[2], 48);
	PA_SetLongintVariable(&args[3], 48);
	PA_ExecuteCommandByID(679, args, 4);
	icon48 = PA_DuplicatePicture(PA_GetPictureVariable(args[1]), 1);
	PA_ClearVariable(&args[1]);
	
	//
	args[1] = PA_CreateVariable(eVK_Picture);
	PA_SetLongintVariable(&args[2], 64);
	PA_SetLongintVariable(&args[3], 64);
	PA_ExecuteCommandByID(679, args, 4);
	icon64 = PA_DuplicatePicture(PA_GetPictureVariable(args[1]), 1);
	PA_ClearVariable(&args[1]);

	//
	args[1] = PA_CreateVariable(eVK_Picture);
	PA_SetLongintVariable(&args[2], 128);
	PA_SetLongintVariable(&args[3], 128);
	PA_ExecuteCommandByID(679, args, 4);
	icon128 = PA_DuplicatePicture(PA_GetPictureVariable(args[1]), 1);
	PA_ClearVariable(&args[1]);
	
	//Extra Large
	args[1] = PA_CreateVariable(eVK_Picture);
	PA_SetLongintVariable(&args[2], 256);
	PA_SetLongintVariable(&args[3], 256);
	PA_ExecuteCommandByID(679, args, 4);
	icon256 = PA_DuplicatePicture(PA_GetPictureVariable(args[1]), 1);
	PA_ClearVariable(&args[1]);
	
	PA_ClearVariable(&args[0]);
	PA_ClearVariable(&args[2]);
	PA_ClearVariable(&args[3]);
	
	getPNG(&icon16,  png16);
	getPNG(&icon32,  png32);
	getPNG(&icon48,  png48);
	getPNG(&icon64,  png64);
	getPNG(&icon128, png128);
	getPNG(&icon256, png256);
	
	PA_DisposePicture(icon16);
	PA_DisposePicture(icon32);
	PA_DisposePicture(icon48);
	PA_DisposePicture(icon64);
	PA_DisposePicture(icon128);
	PA_DisposePicture(icon256);
}

void PICTURE_TO_ICO(sLONG_PTR *pResult, PackagePtr pParams)
{
	//typecheck
	PA_Picture picture = *(PA_Picture *)(pParams[0]);
	
	//convert to png
	PA_Variable args[2];
	args[0] = PA_CreateVariable(eVK_Picture);
	args[1] = PA_CreateVariable(eVK_Unistring);
	PA_SetPictureVariable(&args[0], PA_DuplicatePicture(picture, 1));
	PA_Unistring u = PA_CreateUnistring((PA_Unichar *)".\0p\0n\0g\0\0\0");
	PA_SetStringVariable(&args[1], &u);
	PA_ExecuteCommandByID(1002, args, 2);
	picture = PA_DuplicatePicture(PA_GetPictureVariable(args[0]), 1);
	PA_ClearVariable(&args[0]);
	PA_ClearVariable(&args[1]);
	
	std::vector<uint8_t> png16;
	std::vector<uint8_t> png32;
	std::vector<uint8_t> png48;
	std::vector<uint8_t> png64;
	std::vector<uint8_t> png128;
	std::vector<uint8_t> png256;
	
	getIcons(&picture, png16, png32, png48, png64, png128, png256);
	
	PA_DisposePicture(picture);
	
	//https://msdn.microsoft.com/en-us/library/ms997538.aspx
	//http://hiroshi0945.seesaa.net/article/162310812.html
	
	ICONDIR dir;
	// Setup the icon header
	dir.idReserved = 0; // Must be 0
	dir.idType = 1; // Type 1 = ICON (type 2 = CURSOR)
	dir.idCount = 6; // number of ICONDIRs
	
	size_t headeroffset = sizeof(WORD) * 3;
	
	DWORD icon256offset = headeroffset + sizeof(ICONDIRENTRY) * dir.idCount;
	DWORD icon128offset = icon256offset + png256.size();
	DWORD icon64offset = icon128offset + png128.size();
	DWORD icon48offset = icon64offset + png64.size();
	DWORD icon32offset = icon48offset + png48.size();
	DWORD icon16offset = icon32offset + png32.size();
	
	size_t iconsize = icon16offset + png16.size();
										
	std::vector<uint8_t> ico(iconsize);
	
	ICONDIRENTRY icon256header;
	icon256header.bWidth = 0;
	icon256header.bHeight = 0;
	icon256header.bColorCount = 0;
	icon256header.bReserved = 0;
	icon256header.wPlanes = 1;
	icon256header.wBitCount = 32;
	icon256header.dwBytesInRes = png256.size();
	icon256header.dwImageOffset = icon256offset;
	
	ICONDIRENTRY icon128header;
	icon128header.bWidth = 128;
	icon128header.bHeight = 128;
	icon128header.bColorCount = 0;
	icon128header.bReserved = 0;
	icon128header.wPlanes = 1;
	icon128header.wBitCount = 32;
	icon128header.dwBytesInRes = png128.size();
	icon128header.dwImageOffset = icon128offset;
	
	ICONDIRENTRY icon64header;
	icon64header.bWidth = 64;
	icon64header.bHeight = 64;
	icon64header.bColorCount = 0;
	icon64header.bReserved = 0;
	icon64header.wPlanes = 1;
	icon64header.wBitCount = 32;
	icon64header.dwBytesInRes = png64.size();
	icon64header.dwImageOffset = icon64offset;
	
	ICONDIRENTRY icon48header;
	icon48header.bWidth = 48;
	icon48header.bHeight = 48;
	icon48header.bColorCount = 0;
	icon48header.bReserved = 0;
	icon48header.wPlanes = 1;
	icon48header.wBitCount = 32;
	icon48header.dwBytesInRes = png48.size();
	icon48header.dwImageOffset = icon48offset;
	
	ICONDIRENTRY icon32header;
	icon32header.bWidth = 32;
	icon32header.bHeight = 32;
	icon32header.bColorCount = 0;
	icon32header.bReserved = 0;
	icon32header.wPlanes = 1;
	icon32header.wBitCount = 32;
	icon32header.dwBytesInRes = png32.size();
	icon32header.dwImageOffset = icon32offset;
	
	ICONDIRENTRY icon16header;
	icon16header.bWidth = 16;
	icon16header.bHeight = 16;
	icon16header.bColorCount = 0;
	icon16header.bReserved = 0;
	icon16header.wPlanes = 1;
	icon16header.wBitCount = 32;
	icon16header.dwBytesInRes = png16.size();
	icon16header.dwImageOffset = icon16offset;
	
	memcpy(&ico[0], &dir, headeroffset);
	memcpy(&ico[headeroffset],                             &icon256header, sizeof(ICONDIRENTRY));
	memcpy(&ico[headeroffset +  sizeof(ICONDIRENTRY)],     &icon128header, sizeof(ICONDIRENTRY));
	memcpy(&ico[headeroffset + (sizeof(ICONDIRENTRY) * 2)], &icon64header, sizeof(ICONDIRENTRY));
	memcpy(&ico[headeroffset + (sizeof(ICONDIRENTRY) * 3)], &icon48header, sizeof(ICONDIRENTRY));
	memcpy(&ico[headeroffset + (sizeof(ICONDIRENTRY) * 4)], &icon32header, sizeof(ICONDIRENTRY));
	memcpy(&ico[headeroffset + (sizeof(ICONDIRENTRY) * 5)], &icon16header, sizeof(ICONDIRENTRY));
	
	memcpy(&ico[icon256offset], &png256[0], png256.size());
	memcpy(&ico[icon128offset], &png128[0], png128.size());
	memcpy(&ico[icon64offset ],  &png64[0],  png64.size());
	memcpy(&ico[icon48offset ],  &png48[0],  png48.size());
	memcpy(&ico[icon32offset ],  &png32[0],  png32.size());
	memcpy(&ico[icon16offset ],  &png16[0],  png16.size());
	
	
	C_BLOB Param2;
	Param2.setBytes((const uint8_t *)&ico[0], iconsize);
	Param2.toParamAtIndex(pParams, 2);
}
