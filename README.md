# ALMO


<p align="center">
<img src="https://www.abap34.com/almo_logo.png" width="300px">
</p>


ALMOはC++製の拡張Markdownパーサです。 

ALMOは、WebAseemblyを使ってブラウザ上で完結するサーバ不要の実行およびジャッジ環境を提供します。 

## 機能

ALMOの拡張構文は、

- 競技プログラミング
- データ分析
- 科学技術計算

をはじめとしたプログラミングに関する様々な記事を作成するために設計されています。

### 実行可能コードブロック

ALMOは、実行可能なコードブロックを提供します。

```
:::code
# このコードは自分自身を出力します！　

# 実行して確かめよう

_='_=%r;print(_%%_)';print(_%_)
:::
```

という記法は、ALMOによって変換されて、以下のようなエディタが提供されます。

さらにコードは実行することができ、また実行はユーザのブラウザ上で完結します。

したがって、サンプルプログラムの実行をサーバを用意しなくてもユーザに提供することができます。

![](example/demo_code.gif)

### matplotlib によるグラフ描画

実行可能コードブロックでは、Pythonの主要なライブラリを利用できます。

```
:::loadlib
numpy
matplotlib
:::
```

などとすることで、 `numpy`, `matplotlib` を利用できるようになります。

さらに、`matploblib` によるグラフ描画も可能です。

ソースコード内で、　

```python
import matplotlib
matplotlib.use("module://matplotlib_pyodide.html5_canvas_backend")
```

とすると、通常のように `plt.show()` するだけで出力欄にインタラクティブなプロットを表示できます。

![](example/demo_plot.gif)

### ジャッジシステム

競技プログラミングなどで用いられるジャッジシステムも提供します。

```
:::judge
title=Hello ALMO!　　　                        
sample_in=example/helloalmo/in/sample.txt     
sample_out=example/helloalmo/out/sample.txt  
in=example/helloalmo/in/*.txt                 
out=example/helloalmo/out/*.txt               
:::
```

という記法によって、

- サンプル入力は `example/helloalmo/in/sample.txt` 
- サンプル出力は `example/helloalmo/out/sample.txt`
- 入力ファイルは `example/helloalmo/in/*.txt`
- 出力ファイルは `example/helloalmo/out/*.txt`

と対応したジャッジシステムを自動で構築します。

![](example/demo_judge.gif)



これらは全て[デモページ](https://www.abap34.com/almo.html)で試すことができます。

詳しい記法は[ドキュメント](https://www.abap34.com/almo_document.html)を参照してください。


## 使い方

`almo [options] <markdown file>`

### オプション

以下は全て省略可能です。

- `-o <output file>`: 出力ファイル名を指定します。デフォルトは第一引数の入力ファイル名の拡張子を`.html`にしたものです。
- `-t <theme>` : テーマを指定します。デフォルトは`light`です。`light`または`dark`を指定できます。 
- `-c` : ユーザ定義のCSSファイルを指定します。デフォルトでは `-t` で指定されたテーマのデフォルト仕様である
[dark.css](https://github.com/abap34/ALMO/blob/main/src/dark.css) または [light.css](https://github.com/abap34/ALMO/blob/main/src/light.css) が使用されます。 
- `-d` : デバッグモードにします。デフォルトはオフです。　パースされた結果がJSON形式で標準出力に出力されます。
- `-h` : ヘルプを表示します。


## インストール方法

macOS (Apple Silicon) では、 Homebrew を使ってビルド済みのバイナリをインストールできます。

```bash
brew tap abap34/homebrew-almo
brew install almo
```

それ以外の環境では、 `src/almo.cpp` を以下のコンパイルしてください。

```bash
git clone https://github.com/abap34/ALMO
cd ALMO
g++ -std=c++20 -lcurl -o almo src/almo.cpp
```

`curl` が必要です。

## ドキュメント

[https://www.abap34.com/almo_document.html](https://www.abap34.com/almo_document.html)