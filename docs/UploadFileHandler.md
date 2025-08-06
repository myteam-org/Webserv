# UploadFileHandler

## 概要

`UploadFileHandler`は、HTTP POSTリクエストでファイルアップロードを処理するためのハンドラーです。

## 機能

- POST リクエストによるファイルアップロード機能
- ファイルサイズの検証（デフォルト: 10MB制限）
- 一意なファイル名の生成（タイムスタンプベース）
- JSON形式のレスポンス返却
- 適切なHTTPステータスコードの設定

## レスポンス

### 成功時 (201 Created)
```json
{
  "status": "success",
  "filename": "1690000000_uploaded_file",
  "size": 1024
}
```

### エラー時
- 405 Method Not Allowed: POST以外のメソッド
- 400 Bad Request: 空のリクエストボディ
- 413 Payload Too Large: ファイルサイズ制限超過
- 500 Internal Server Error: ファイル保存失敗

## 使用方法

```cpp
#include "http/handler/file/upload.hpp"

// DocumentRootConfigを設定
DocumentRootConfig config;
config.setRoot("/path/to/upload/directory");

// UploadFileHandlerを作成
UploadFileHandler handler(config);

// リクエストを処理
Either<IAction *, Response> result = handler.serve(request);
```

## テスト

`test/http/handler/file/upload_test.cpp` に以下のテストケースが含まれています：

- `HandlePostRequestWithFileData`: 正常なファイルアップロードのテスト
- `RejectNonPostMethods`: POST以外のメソッドを拒否するテスト
- `RejectEmptyBody`: 空のボディを拒否するテスト