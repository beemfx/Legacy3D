#include <stdio.h>
#include <conio.h>
#include "var_mgr.h"


using namespace var_sys;

int main()
{
	CVarMgr varMgr(30);
	
	varMgr.Register<var_word>(_T("var1"), 1, 0);
	varMgr.Register<var_word>(_T("var2"), 1, 0);
	varMgr.Register<var_word>(_T("var3"), 1, 0);
	varMgr.Register<var_word>(_T("var4"), 1, 0);
	varMgr.Register<var_word>(_T("var5"), 1, 0);
	varMgr.Register<var_word>(_T("var6"), 1, 0);
	varMgr.Register<var_word>(_T("var7"), 1, 0);
	varMgr.Register<var_word>(_T("var8"), 1, 0);
	varMgr.Register<var_word>(_T("var9"), 1, 0);
	varMgr.Register<var_word>(_T("var10"), 1, 0);
	varMgr.Register<var_word>(_T("var11"), 1, 0);
	varMgr.Register<var_float>(_T("var12"), 5.0f, CVar::F_ROM);
	varMgr.Register<var_long>(_T("var13"), -20, 0);
	varMgr.Register<var_word>(_T("var14"), 1, 0);
	varMgr.Register<var_word>(_T("var15"), 1, 0);
	varMgr.Register<var_word>(_T("var16"), 1, 0);
	varMgr.Register<var_word>(_T("var17"), 1, 0);
	varMgr.Register<var_word>(_T("var18"), 1, 0);
	varMgr.Register<var_word>(_T("var19"), 1, 0);
	varMgr.Register<var_word>(_T("var20"), 1, 0);
	varMgr.Register(_T("m_NewVar"), _T("Hello"), 0);
	varMgr.Register<var_word>(_T("var21"), 1, 0);
	
	varMgr.Define(_T("TRUE"), (var_word)1);
	varMgr.Define(_T("FALSE"), (var_word)0);
	varMgr.Define(_T("TEN"), (var_word)10);
	
	
	varMgr[_T("var1")].Set(_T("Hi"));
	
	lg_var var = varMgr[_T("var20")];
	
	//This should not be able to register since it's already registered.
	lg_var othervar = varMgr.Register<var_word>(_T("var20"), 30, 0);
	
	var = _T("True");
	
	var = varMgr[_T("m_NEwVar")];
	
	var = _T("Ten");
	
		wprintf(_T("The value of %s is: (%f) (%i) (%u) (0x%08X) (%s)\n"), 
			var.Name(),
			var.Float(),
			var.Long(),
			var.Word(),
			var.Word(),
			var.String());
		
	varMgr.DebugPrintList();	

	_getch();
	return 0;
}
