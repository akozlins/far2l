#include <algorithm>
#include <utils.h>
#include <WordExpansion.h>
#include <StringConfig.h>
#include "../DialogUtils.h"
#include "../../Globals.h"


/*                                                         62
345                      28         39                   60  64
 ================ SFTP Protocol options =====================
| [ ] Private key file path:                                 |
| [EDIT....................................................] |
| Max read block size, bytes:                 [9999999]      |
| Max write block size, bytes:                [9999999]      |
| [ ] Enable TCP_NODELAY option (libssh >= v0.8.0)           |
| [ ] Enable sandbox (if you dont trust server)              |
|------------------------------------------------------------|
|             [  OK    ]        [        Cancel       ]      |
 ============================================================
    6                     29       38                      60
*/

class ProtocolOptionsSFTP : protected BaseDialog
{
	int _i_ok = -1, _i_cancel = -1;
	int _i_privkey_enable = -1, _i_privkey_path = -1;
	int _i_max_read_block_size = -1, _i_max_write_block_size = -1;
	int _i_tcp_nodelay = -1;
	//int _i_enable_sandbox = -1;

	bool _keypath_enabled = true, _ok_enabled = true;

	void UpdateEnableds(bool due_privkey_enable_clicked)
	{
		bool ok_enabled = true, keypath_enabled = false;
		if (IsCheckedDialogControl(_i_privkey_enable)) {
			ok_enabled = false;
			keypath_enabled = true;
			std::string str;
			TextFromDialogControl(_i_privkey_path, str);
			if (str.empty() && due_privkey_enable_clicked) {
				str = "~/.ssh/id_rsa";
				TextToDialogControl(_i_privkey_path, str);
			}
			WordExpansion we(str);
			for (const auto &str : we) {
				struct stat s{};
				if (!str.empty() && stat(str.c_str(), &s) == 0 && S_ISREG(s.st_mode)) {
					ok_enabled = true;
					break;
				}
			}
		}

		if (ok_enabled != _ok_enabled) {
			_ok_enabled = ok_enabled;
			SetEnabledDialogControl(_i_ok, ok_enabled);
		}

		if (keypath_enabled != _keypath_enabled) {
			_keypath_enabled = keypath_enabled;
			SetEnabledDialogControl(_i_privkey_path, keypath_enabled);
		}
	}

	LONG_PTR DlgProc(int msg, int param1, LONG_PTR param2)
	{
		if ( msg == DN_INITDIALOG
		|| (msg == DN_BTNCLICK && param1 == _i_privkey_enable)
		|| (msg == DN_EDITCHANGE && param1 == _i_privkey_path) ) {
			UpdateEnableds(msg == DN_BTNCLICK && param1 == _i_privkey_enable);
		}

		return BaseDialog::DlgProc(msg, param1, param2);
	}

public:
	ProtocolOptionsSFTP()
	{
		_di.Add(DI_DOUBLEBOX, 3,1,64,9, 0, MSFTPOptionsTitle);
		_di.SetLine(2);
		_i_privkey_enable = _di.AddAtLine(DI_CHECKBOX, 5,62, 0, MSFTPPrivateKeyPath);
		_di.NextLine();
		_i_privkey_path = _di.AddAtLine(DI_EDIT, 5,62, 0, "");

		_di.NextLine();
		_di.AddAtLine(DI_TEXT, 5,50, 0, MSFTPMaxReadBlockSize);
		_i_max_read_block_size = _di.AddAtLine(DI_FIXEDIT, 51,60, DIF_MASKEDIT, "32768", "9999999999");

		_di.NextLine();
		_di.AddAtLine(DI_TEXT, 5,50, 0, MSFTPMaxWriteBlockSize);
		_i_max_write_block_size = _di.AddAtLine(DI_FIXEDIT, 51,60, DIF_MASKEDIT, "32768", "9999999999");

		_di.NextLine();
		_i_tcp_nodelay = _di.AddAtLine(DI_CHECKBOX, 5,60, 0, MSFTPTCPNodelay);

	//	_di.NextLine();
	//	_i_enable_sandbox = _di.AddAtLine(DI_CHECKBOX, 5,60, 0, MSFTPEnableSandbox);

		_di.NextLine();
		_di.AddAtLine(DI_TEXT, 4,61, DIF_BOXCOLOR | DIF_SEPARATOR);

		_di.NextLine();
		_i_ok = _di.AddAtLine(DI_BUTTON, 7,29, DIF_CENTERGROUP, MOK);
		_i_cancel = _di.AddAtLine(DI_BUTTON, 38,58, DIF_CENTERGROUP, MCancel);

		SetFocusedDialogControl(_i_ok);
		SetDefaultDialogControl(_i_ok);
	}


	void Configure(std::string &options)
	{
		StringConfig sc(options);
		SetCheckedDialogControl( _i_privkey_enable, sc.GetInt("PrivKeyEnable", 0) != 0);
		TextToDialogControl(_i_privkey_path, sc.GetString("PrivKeyPath"));
		LongLongToDialogControl(_i_max_read_block_size, std::max((int)512, sc.GetInt("MaxReadBlock", 32768)));
		LongLongToDialogControl(_i_max_write_block_size, std::max((int)512, sc.GetInt("MaxWriteBlock", 32768)));
		SetCheckedDialogControl(_i_tcp_nodelay, sc.GetInt("TcpNoDelay", 0) != 0);
	//	SetCheckedDialogControl(_i_enable_sandbox, sc.GetInt("Sandbox", 0) != 0);
		if (Show(L"ProtocolOptionsSFTP", 6, 2) == _i_ok) {
			sc.SetInt("PrivKeyEnable", IsCheckedDialogControl(_i_privkey_enable) ? 1 : 0);
			std::string str;
			TextFromDialogControl(_i_privkey_path, str);
			sc.SetString("PrivKeyPath", str);
			sc.SetInt("MaxReadBlock", std::max((int)512, (int)LongLongFromDialogControl(_i_max_read_block_size)));
			sc.SetInt("MaxWriteBlock", std::max((int)512, (int)LongLongFromDialogControl(_i_max_write_block_size)));
			sc.SetInt("TcpNoDelay", IsCheckedDialogControl(_i_tcp_nodelay) ? 1 : 0);
	//		sc.SetInt("Sandbox", IsCheckedDialogControl(_i_enable_sandbox) ? 1 : 0);
			options = sc.Serialize();
		}
	}
};

void ConfigureProtocolSFTP(std::string &options)
{
	ProtocolOptionsSFTP().Configure(options);
}
