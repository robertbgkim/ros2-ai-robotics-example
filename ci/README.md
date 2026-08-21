# CI 설정

`github-actions-ci.yml`은 GitHub Actions 워크플로 파일입니다.

이 저장소에 `.github/workflows/`로 바로 두지 않은 이유는 워크플로 파일을 밀어 넣으려면
푸시하는 토큰에 `workflow` 스코프가 있어야 하기 때문입니다. 스코프를 갖춘 계정에서
아래처럼 옮기면 그대로 동작합니다.

```bash
mkdir -p .github/workflows
cp ci/github-actions-ci.yml .github/workflows/ci.yml
git add .github/workflows/ci.yml
```
