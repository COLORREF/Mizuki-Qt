"""
URL 路由配置
"""
from django.urls import path, re_path
from django.views.static import serve
from pathlib import Path
from . import views

WASM_ROOT = Path(__file__).resolve().parent.parent / "www"

urlpatterns = [
    # 首页：返回 blog 入口 HTML
    # 示例：GET /
    path("", views.index_page),

    # 屏蔽浏览器安全探针请求，直接返回 204 No Content
    # 示例：GET /.well-known/traffic-advice
    re_path(r"^\.well-known/", views.well_known),

    # 查询指定图片分类目录下的最大数字编号
    # 示例：GET /api/images/max_id/Homepage/  →  {"category": "Homepage", "max_id": 4}
    path("api/images/max_id/<str:category>/", views.category_max_id),

    # 兜底静态文件托管：未被以上路由匹配的路径均从 www/ 目录提供静态文件
    # 示例：GET /Images/Homepage/1.jpg  →  返回 www/Images/Homepage/1.jpg
    re_path(r"^(?P<path>.+)$", serve, {"document_root": WASM_ROOT}),
]
