#!/bin/bash

# -------- CONFIG --------
GITHUB_USER="Phanhuynh0210"
GITHUB_REPO="TTNT"
GITHUB_TOKEN=""  # <-- Đổi token thật ở đây
BUILD_DIR=".pio/build/yolo_uno"
BIN_FILES=("firmware.bin" "partitions.bin")
# ------------------------

# Check các công cụ cần thiết
command -v jq >/dev/null 2>&1 || { echo >&2 "[ERROR] jq not installed."; exit 1; }
command -v git >/dev/null 2>&1 || { echo >&2 "[ERROR] git not installed."; exit 1; }

# Kiểm tra file tồn tại
for FILE in "${BIN_FILES[@]}"; do
  if [ ! -f "$BUILD_DIR/$FILE" ]; then
    echo "[ERROR] File $FILE not found in $BUILD_DIR!"
    exit 1
  fi
done

# Lấy tag mới nhất
CURRENT_VERSION=$(git tag --sort=-v:refname | head -n 1)

if [[ -z "$CURRENT_VERSION" ]]; then
  CURRENT_VERSION="v0.0.0"
fi

# Tăng phiên bản nhỏ (patch)
IFS='.' read -r MAJOR MINOR PATCH <<<"${CURRENT_VERSION//v/}"
PATCH=$((PATCH + 1))
NEW_VERSION="v$MAJOR.$MINOR.$PATCH"

echo "==> Creating new version: $NEW_VERSION"

# Tạo tag mới
git tag "$NEW_VERSION"
git push origin "$NEW_VERSION"

# Tạo release nếu chưa có
RELEASE_ID=$(curl -s \
  -H "Authorization: token $GITHUB_TOKEN" \
  https://api.github.com/repos/$GITHUB_USER/$GITHUB_REPO/releases/tags/$NEW_VERSION \
  | jq -r .id)

if [[ "$RELEASE_ID" == "null" || "$RELEASE_ID" == "" ]]; then
  echo "==> Creating new release $NEW_VERSION..."
  RESPONSE=$(curl -s -X POST \
    -H "Authorization: token $GITHUB_TOKEN" \
    -H "Content-Type: application/json" \
    -d "{\"tag_name\":\"$NEW_VERSION\",\"name\":\"Firmware $NEW_VERSION\",\"body\":\"Auto uploaded by PlatformIO\",\"draft\":false,\"prerelease\":false}" \
    https://api.github.com/repos/$GITHUB_USER/$GITHUB_REPO/releases)
  RELEASE_ID=$(echo "$RESPONSE" | jq -r .id)
fi

# Lặp qua từng file để xóa asset cũ (nếu có) và upload
for FILE in "${BIN_FILES[@]}"; do
  ASSET_ID=$(curl -s \
    -H "Authorization: token $GITHUB_TOKEN" \
    https://api.github.com/repos/$GITHUB_USER/$GITHUB_REPO/releases/$RELEASE_ID/assets \
    | jq ".[] | select(.name==\"$FILE\") | .id")

  if [[ "$ASSET_ID" != "" ]]; then
    echo "==> Deleting existing asset: $FILE"
    curl -s -X DELETE \
      -H "Authorization: token $GITHUB_TOKEN" \
      https://api.github.com/repos/$GITHUB_USER/$GITHUB_REPO/releases/assets/$ASSET_ID
  fi

  echo "==> Uploading $FILE..."
  curl -s -X POST \
    -H "Authorization: token $GITHUB_TOKEN" \
    -H "Content-Type: application/octet-stream" \
    --data-binary @"$BUILD_DIR/$FILE" \
    "https://uploads.github.com/repos/$GITHUB_USER/$GITHUB_REPO/releases/$RELEASE_ID/assets?name=$FILE"
done

echo "Done uploading firmware for version $NEW_VERSION."
