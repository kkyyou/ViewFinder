#pragma once

#include <Windows.h>


/*------------------------
		CFontInfo

      폰트 정보 클래스
--------------------------*/

class CFontInfo
{
private:
	CFontInfo();
	~CFontInfo();

public:
	static CFontInfo* GetInstance();
	static void		  DeleteInstance();

	HFONT			  GetDefaultFont() const;

private:
	static CFontInfo* m_instance;
	HFONT			  m_defaultFont;
};

