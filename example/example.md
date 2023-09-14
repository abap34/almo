---
title: ALMO デモページ
date: 2023-08-10
author: abap34
twitter_id: abap34
github_id: abap34
mail: abap0002@gmail.com
tag: ["ALMO", "Python", "Markdown", "Pyodide", "WebAssembly"]
author: abap34
---

## はじめに


ALMOは拡張Markdownパーサです。

ALMOの拡張構文の最大の特徴、そして売りは、**実行・ジャッジ可能なコードブロック**です。

下にいかにも入力可能そうなエディタと、ボタンがあります。

そこに、入力を受け取って、 `Hello {入力された文字列}!` という文字列を出力するPythonのプログラムを書いてみましょう。

ちなみに、

- 入力は`input()`
- 文字列結合は`+`
- 出力は`print()`

でできます。


:::code
title=Hello ALMO!
sample_in=example/helloalmo/in/sample.txt
sample_out=example/helloalmo/out/sample.txt
in=example/helloalmo/in/*.txt
out=example/helloalmo/out/*.txt
:::


ボタンの
- "Run Sample" ボタンでサンプルに対して実行
- "Submit" ボタンで複数のテストケースでジャッジ

ができます。

このPythonの実行は、サーバーで実行されているのではなく、

**WebAseemblyを使ってユーザのブラウザ上で完結しています。**


具体的にはALMOは、[Pyodide](https://pyodide.org/en/stable/) を使ってブラウザ上でPythonのコードを実行しています。


また、ジャッジコードもALMOに組み込まれていて、全ての処理がブラウザ上で完結しています！

(つまり一度読み込みさえ終わればオフライン環境でも動作します。)


## 全部入りHTML

ALMOは**全部入り**のHTMLファイルを出力するパーサです。

上のブロックは

```
:::code
title=Hello ALMO!
sample_in=example/helloalmo/in/sample.txt
sample_out=example/helloalmo/out/sample.txt
in=example/helloalmo/in/*.txt
out=example/helloalmo/out/*.txt
:::
```

と書くだけで作られ、そして全ての出入力ファイルがHTMLファイルに埋め込まれて、自動でジャッジが構築されます。

普通のコードブロックも作れます、もちろん

```
def hello():
    print("Hello ALMO!")
```


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

全部入りのHTMLファイルなので、そのまま置くだけで動作します。

記法の詳細については [レポジトリのREADME.md](https://github.com/abap34/ALMO)を参照してください。


