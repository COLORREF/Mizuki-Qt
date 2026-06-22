"""
API 视图
"""
import re
from pathlib import Path
from django.http import HttpResponse, Http404

WASM_ROOT = Path(__file__).resolve().parent.parent / "www"


def index_page(request):
    """博客首页"""
    index_path = WASM_ROOT / "index.html"
    if not index_path.is_file():
        raise Http404("index.html not found")
    return HttpResponse(
        index_path.read_bytes(),
        content_type="text/html; charset=utf-8",
    )


def well_known(request, path=None):
    """屏蔽浏览器探针请求（.well-known/*）"""
    return HttpResponse(status=204)


def category_max_id(request, category: str):
    """查询指定分类下最大图片编号

    请求路径：/api/images/max_id/Homepage/
    返回：纯数字（如 "4"），Content-Type: text/plain

    支持扩展：Images/ 下新增任意分类目录（如 Avatar / Gallery）即可自动支持。
    """
    # 安全检查：category 只能是合法目录名，防止路径穿越
    if not re.fullmatch(r"[A-Za-z0-9_-]+", category):
        return HttpResponse("Invalid category name", status=400)

    images_dir = WASM_ROOT / "Images" / category
    if not images_dir.is_dir():
        return HttpResponse("0", content_type="text/plain")

    max_id = 0
    for f in images_dir.iterdir():
        if not f.is_file():
            continue
        stem = f.stem  # 不带后缀的文件名
        try:
            num = int(stem)
            if num > max_id:
                max_id = num
        except ValueError:
            continue  # 跳过非数字命名的文件

    return HttpResponse(str(max_id), content_type="text/plain")
