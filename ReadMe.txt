■ BkAddressGroupExtract とは

アドレスグループを展開してグループメンバの個別メールアドレスに置換する Becky! 
Internet Mail 用プラグインです。
  ex. アドレスグループ「Personal」->「友達」に「日本 太郎」と「日本 花子」が登
      録されている場合

    To: @"Personal:友達" と入力すると次のように展開されます。
    To: 日本 太郎 <tnihon@nihon.jp>, 日本 花子 <hnihon@nihon.jp>

展開するメールアドレスのフォーマットを以下のいずれかの形式で設定できます。
  1. user@domain
  2. NAME <user@domain>
  3. user@domain (NAME)
※ 3. の宛先形式は従来仕様のため、使用しないことが望ましい(SHOULD)とされていま
   す。

■ インストール方法

1. Becky! を起動している場合は停止させます
2. Becky! の PlugIns フォルダに「BkAddressGroupExtract.dll」をコピーまたは移動
   させてください
3. Becky! を起動させ、「BkAddressGroupExtract」の読み込みを許可してください


■ アンインストール方法

Becky! の PlugIns フォルダから「BkAddressGroupExtract.dll」を削除してください。
設定ファイルも削除する場合は同フォルダから「BkAddressGroupExtract.ini」を削除し
てください。
※Windows Vista 以降では設定ファイルは Virtual Store フォルダ配下にある Becky! 
  の PlugIns フォルダに保存されます。
  Virtual Store は [ユーザフォルダ] -> [AppData] -> [Local] -> [Virtual Store]
  に存在します。

レジストリに設定は残していませんので、DLL 本体と設定ファイルを削除すれば完全に
アンインストールされます。


■ 改変履歴

0.1 初版
0.2 全返信でメールを作成すると Cc に冗長なアドレスが追加される問題を修正
1.0 展開したアドレスをフォーマットする機能を追加

