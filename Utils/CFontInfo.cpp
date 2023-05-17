#include "CFontInfo.h"

#include <stdio.h>

#include "../Define/UiDefine.h"

CFontInfo* CFontInfo::m_instance = nullptr;

CFontInfo::CFontInfo()
{
	m_defaultFont = CreateFont(UI_SIZE_15PX, 0, 0, 0,
		FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Arial"));
}

CFontInfo::~CFontInfo()
{
}

CFontInfo* CFontInfo::GetInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new CFontInfo();
	}

	return m_instance;
}

void CFontInfo::DeleteInstance()
{
	if (m_instance)
	{
		delete m_instance;
		m_instance = nullptr;
	}
}

HFONT CFontInfo::GetDefaultFont() const
{
	return m_defaultFont;
}

