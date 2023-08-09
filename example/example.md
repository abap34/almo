---
title: ALMO デモページ
date: 2023-08-10
author: abap34
---

# ALMO デモページ

## はじめに

ALMOは、拡張Markdownを受け取り実行 & ジャッジ環境同梱の単一のHTMLファイルを出力します。

ALMOは基本的なMarkdownのパーサの機能を内包していますが、

ALMOの最大の特徴、そして売りは、**実行・ジャッジ可能なコードブロック**です。

下にいかにも入力可能そうなエディタと、ボタンがあります。

そこに、

入力を受け取って、 `Hello {入力された文字列}!` という文字列を出力するプログラムを書いてみましょう。


なお、Pythonの標準入力は`input()`で受け取ることができます。
文字列結合は`+`で行え、出力は`print()`で行えます。



:::code
title=Hello ALMO!
sample_in=example/helloalmo/in/sample.txt
sample_out=example/helloalmo/out/sample.txt
in=example/helloalmo/in/*.txt
out=example/helloalmo/out/*.txt
:::




プログラムが書けたら、"Run Sample"ボタンを押してみましょう。
すると、「サンプルの入力」に対する実行結果が、「出力」に表示されます。

ただしいプログラムが書けたら、"Submit"ボタンを押してみましょう。

すると、複数のテストケースに対して正しい出力がなされているかチェックされます。

無事に正しいコードが書かれていれば左上のステータスが"AC(Accepted)"となったはずです！


このように、ALMOはプログラムを書いて実行し、ジャッジすることができます。
そして、この実行は、 **サーバー側ではなく、ユーザーのブラウザ側で完結しています。**

ALMOは、[Pyodide](https://pyodide.org/en/stable/) を使ってブラウザ上でPythonのコードを実行することができます。

また、ジャッジコードも組み込まれており、全ての処理がブラウザ上で完結しています！
(つまり一度読み込みさえ終わればオフライン環境でも動作します.)

そしてこのコードブロックは


```
:::code
title=Hello ALMO!
sample_in=example/helloalmo/in/sample.txt
sample_out=example/helloalmo/out/sample.txt
in=example/helloalmo/in/*.txt
out=example/helloalmo/out/*.txt
:::
```


と書くだけで作ることができます。


このような記述を含むMarkdownファイルをALMOに渡すだけで、ALMOは全ての入出力ファイルをHTMLファイルに埋め込んで、自動でジャッジが構築されます！

つまり、ALMOは**「全部入り」のHTMLファイルを出力する**パーサです！
(他にも、たとえインターネット上のファイルでも、画像は自動でBase64に変換され、HTMLファイルに埋め込まれます。)

![マンドリル](https://imagingsolution.net/wordpress/wp-content/uploads/2011/03/Mandrill.png)



## インストール方法・使い方

ALMOは、Homebrewを使ってインストールすることができます。

```
brew tap abap34/homebrew-almo
brew install almo
```

インストールが完了したら、MarkdownファイルをALMOに渡してみましょう。

```
almo example/example.md > index.html
```

結果は標準出力に吐かれるので、適宜リダイレクトしてください。

HTMLファイルは単体で動作します！

あとは自分のサイトに配置したり、配布して、ぜひ使ってください！

記法の詳細については [レポジトリのREADME.md](https://github.com/abap34/ALMO)を参照してください。



