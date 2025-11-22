# ベンチマーク追跡セットアップガイド

このドキュメントでは、github-action-benchmarkを使った継続的ベンチマーク追跡の設定方法を説明します。

## 前提条件

1. GitHubリポジトリに十分な権限があること
2. GitHub Pagesの機能が利用可能であること
3. Google Benchmarkでベンチマークが実装されていること

## セットアップ手順

### 1. GitHub Pagesの有効化

1. リポジトリの Settings タブを開く
2. 左サイドバーの "Pages" を選択
3. Source を "GitHub Actions" に設定
4. Save ボタンをクリック

### 2. 必要な権限の設定

リポジトリの Settings → Actions → General で以下を設定:

- **Workflow permissions**: "Read and write permissions" を選択
- **Allow GitHub Actions to create and approve pull requests**: チェックを入れる

### 3. ワークフローファイルの確認

以下のファイルが正しく配置されていることを確認:

- `.github/workflows/ci.yml` - メインのCIワークフロー
- `.github/workflows/benchmark-pages.yml` - ベンチマーク結果のPages配信

### 4. 初回実行

1. mainブランチにコミットをプッシュ
2. Actions タブでワークフローの実行を確認
3. 数分後、GitHub Pagesにベンチマーク結果が表示される

## ベンチマーク結果の確認

### GitHub Pages URL

ベンチマーク結果は以下のURLで確認できます:
```
https://yourusername.github.io/almo/dev/bench/
```

### 表示される情報

- **パフォーマンスグラフ**: 時系列でのベンチマーク結果
- **各ベンチマーク詳細**: 個別のベンチマーク結果
- **回帰検出**: 性能低下が検出された場合のハイライト

## アラート設定

### しきい値の調整

`.github/workflows/ci.yml` で以下のパラメータを調整可能:

```yaml
# mainブランチでのアラートしきい値 (200% = 2倍遅くなったらアラート)
alert-threshold: '200%'

# プルリクエストでのアラートしきい値 (150% = 1.5倍遅くなったらアラート)  
alert-threshold: '150%'
```

### アラート方法

- **コミットコメント**: 性能低下が検出されたコミットにコメントが追加
- **プルリクエストコメント**: PRでの性能比較結果をコメントで表示
- **Job Summary**: GitHub Actions の Summary 画面での詳細表示

## カスタマイズ

### ベンチマーク実行時間の調整

より正確な結果を得るため、実行時間を調整可能:

```yaml
# 短時間での実行 (開発時)
--benchmark_min_time=0.1s

# 標準的な実行時間
--benchmark_min_time=0.3s  

# 高精度な実行 (リリース前)
--benchmark_min_time=1.0s
```

### チャート表示の調整

```yaml
# 表示するデータポイント数の制限
max-items-in-chart: 50

# ベンチマークデータの保存場所
benchmark-data-dir-path: dev/bench
```

## トラブルシューティング

### よくある問題

1. **GitHub Pagesが表示されない**
   - Settings → Pages で GitHub Actions が選択されているか確認
   - ワークフローが正常に完了しているか確認

2. **ベンチマーク結果が更新されない**
   - mainブランチでのコミットが必要
   - ワークフローがエラーなく完了しているか確認

3. **アラートが機能しない**
   - GitHub Token の権限が適切か確認
   - alert-threshold の値が適切か確認

### ログの確認

GitHub Actions の詳細ログで以下を確認:

1. ベンチマーク実行の成功/失敗
2. JSON出力ファイルの生成
3. GitHub Pagesへのデプロイ状況

## 高度な設定

### 複数のベンチマークスイート

異なる種類のベンチマークを分けて追跡する場合:

```yaml
- name: Store CPU benchmark result
  uses: benchmark-action/github-action-benchmark@v1
  with:
    name: CPU Performance
    tool: 'googlecpp'
    output-file-path: cpu_benchmark.json
    
- name: Store Memory benchmark result  
  uses: benchmark-action/github-action-benchmark@v1
  with:
    name: Memory Performance
    tool: 'googlecpp'
    output-file-path: memory_benchmark.json
```

### 条件付き実行

特定の条件でのみベンチマークを実行:

```yaml
- name: Run benchmarks
  if: contains(github.event.head_commit.message, '[benchmark]')
  run: ./builddir/bench_almo --benchmark_format=json --benchmark_out=results.json
```

これらの設定により、継続的なパフォーマンス監視と改善のサイクルを確立できます。