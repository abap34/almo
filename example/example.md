---
title: ALMO デモページ
date: 2023-08-10
author: abap34
twitter_id: abap34
github_id: abap34
mail: abap0002@gmail.com
ogp_url: https://www.abap34.com/almo_logo.jpg
tag: [ALMO, Python, Markdown, Pyodide, WebAssembly]
author: abap34
url: abap34.com
site_name: abap34.com
twitter_site: abap34
---


## はじめに

### Meet ALMO!

ALMOは拡張Markdownパーサ / 静的サイトジェネレータ です。 入力として Markdownファイルを受け取り、単一の HTMLファイルを出力します。

ALMOの拡張構文の最大の特徴は、 **実行環境を含むページを作成できる** ことです。

下にいかにも入力可能そうなエディタと、ボタンがあります。

そこに、入力を受け取って、 `Hello {入力された文字列}!` という文字列を出力するPythonのプログラムを書いてみましょう。

(Pythonの文字列結合は `+` でできます。)

:::judge
title=Hello ALMO!
sample_in=example/helloalmo/in/sample.txt
sample_out=example/helloalmo/out/sample.txt
in=example/helloalmo/in/*.txt
out=example/helloalmo/out/*.txt
source=example/input.py
:::



コードができたら、下のボタンでジャッジをしてみましょう！


ボタンの
- "Run Sample" ボタンでサンプルに対して実行
- "Submit" ボタンで複数のテストケースでテスト

ができます。 ACを目指してコードを変更してみましょう！


さて、このPythonの実行は、サーバーで実行されているのではなく、

**WebAseemblyを使ってユーザのブラウザ上で完結しています。**


具体的にはALMOは、[Pyodide](https://pyodide.org/en/stable/) を使ってブラウザ上でPythonのコードを実行しています。


また、ジャッジを行うコード・テストケースも、全て HTMLファイルに埋め込まれていて、
全ての処理がクライアント側で完結しています。

(つまり一度読み込みさえ終わればオフライン環境でも動作します。)


## ALMOの機能

### キホンのキ

ALMO は、競技プログラミング・データ分析・科学技術計算などのプログラミングに関連する記事を
簡単に執筆できるように設計されています。

通常の Markdown記法のようにコードブロックを作ることもで画像を表示することもできますし、


```python
def hello():
    print("Hello ALMO!")

if __name__ == "__main__":
    hello()
```

![マンドリル](https://imagingsolution.net/wordpress/wp-content/uploads/2011/03/Mandrill.png)

単に実行可能なコードブロックも書くことができます。

:::code
def kuku():
    print('+----' * 9 + '+')
    for i in range(1, 10):
        row = '|'
        for j in range(1, 10):
            row += '{:3d} |'.format(i * j) 
        print(row)
        print('+----' * 9 + '+')

kuku()
:::


また、デフォルトテーマでは証明などの環境や注意・警告などのブロックもサポートされており、もちろんカスタマイズ可能です。

:::definition

関数列 $\{f_n\}$ が各点収束するとは、任意の $x$ に対して $\displaystyle \lim_{n \to \infty} f_n(x)$ が存在することである。

つまり、

$$
\forall \varepsilon > 0, \forall x \in X, \exists N \in \mathbb{N}, \forall n \geq N, |f_n(x) - f(x)| < \varepsilon
$$

が成り立つことである。


:::warning
ここで、 $N$ は 各 $\varepsilon, x$ に対してとってこれることに注意しよう。

各 $\varepsilon$ に対してある $N$ が存在して、 各 $x$ に対して $n \geq N$ ならば $|f_n(x) - f(x)| < \varepsilon$ が成り立つときは、
関数列 $\{f_n\}$ は **一様収束する** という。

:::

:::


### ライブラリの読み込み

さらに、主要なPythonのライブラリも実行環境に読み込ませることが可能です。

```
:::loadlib
numpy
scipy
matplotlib
:::
```

と書くと、 `numpy`, `scipy`, `matplotlib` が `import` 可能になり、コードに含めることができます。

:::loadlib
numpy
scipy
matplotlib
:::

### matplotlibのサポート

`matplotlib` を使ったプロットをサポートしています。

事前に `matplotlib` を読み込んでおいて、 `matplotlib.use("module://matplotlib_pyodide.html5_canvas_backend")` を追加すると、
通常のように `plt.show()` とするだけで、ブラウザ上でプロットを表示することができます。

:::code
import matplotlib.pyplot as plt

import matplotlib
matplotlib.use("module://matplotlib_pyodide.html5_canvas_backend")

import numpy as np
from scipy.integrate import solve_ivp
import matplotlib.pyplot as plt

# 振動子の運動方程式
def harmonic_oscillator(t, y, k, m):
    x, v = y
    dxdt = v
    dvdt = -k * x / m
    return [dxdt, dvdt]

# パラメータ設定
k = 1.0  # 弾性定数
m = 1.0  # 質量
initial_conditions = [1.0, 0.0]  # 初期位置と初速度

# 時間の範囲
t_span = (0.0, 10.0)

# 数値的に運動方程式を解く
solution = solve_ivp(harmonic_oscillator, t_span, initial_conditions, args=(k, m), dense_output=True)

# 時間の範囲を指定して解を評価
t_eval = np.linspace(0, 10, 1000)
y_eval = solution.sol(t_eval)

# 結果を可視化
plt.figure(figsize=(8, 4))
plt.plot(t_eval, y_eval[0], label='位置')
plt.plot(t_eval, y_eval[1], label='速度')
plt.xlabel('時間')
plt.ylabel('位置と速度')
plt.legend()
plt.title('弾性振動子の振動')
plt.grid(True)
plt.show()
:::


## インストール

ALMOは、Homebrewを使ってインストールすることができます。

```bash
brew tap abap34/homebrew-almo
brew install almo
```

インストールが完了したら、MarkdownファイルをALMOに渡してみましょう。

```bash
almo example/example.md -o example.html
```

記法の詳細については [レポジトリのREADME.md](https://github.com/abap34/ALMO)を参照してください。

## ライブラリ

### 1. Visual Studio Codeでのプレビュー [https://github.com/abap34/ALMO-extension](https://github.com/abap34/ALMO-extension)

Visual Studio Code上で、執筆中の記事のプレビューが可能です.


![](https://github.com/abap34/ALMO-extension/raw/main/assets/almo-ext-demo.gif)


### 2. GitHub Pages へのデプロイ [https://github.com/abap34/ALMO-blog-template](https://github.com/abap34/ALMO-blog-template)


GitHub Pages を使って、簡単にブログを構築することができます。

![](https://github.com/abap34/ALMO-blog-template/raw/main/assets/index-example.png)