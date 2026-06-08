import argparse
import os
import sys
import tempfile
import urllib.parse

import cv2
import requests


def resolve_default_video_path() -> str:
    base_dir = getattr(sys, "_MEIPASS", os.path.dirname(os.path.abspath(__file__)))
    return os.path.join(base_dir, "thugmain.mp4")


def download_file(url: str, dest_path: str, chunk_size: int = 1024 * 64) -> None:
    resp = requests.get(url, stream=True, timeout=30)
    resp.raise_for_status()
    with open(dest_path, "wb") as f:
        for chunk in resp.iter_content(chunk_size=chunk_size):
            if chunk:
                f.write(chunk)


def display_video_loop(video_path: str) -> None:
    if not os.path.isfile(video_path):
        print(f"Error: video file not found: {video_path}")
        sys.exit(1)

    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        print("Error: Could not open video.")
        sys.exit(1)

    window_name = "Video"
    cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)
    cv2.setWindowProperty(window_name, cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)

    fps = cap.get(cv2.CAP_PROP_FPS) or 25.0
    delay = max(1, int(1000.0 / fps))

    while True:
        ret, frame = cap.read()
        if not ret:
            # loop back to start
            cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
            continue

        cv2.imshow(window_name, frame)

        if cv2.getWindowProperty(window_name, cv2.WND_PROP_VISIBLE) < 1:
            break

        key = cv2.waitKey(delay) & 0xFF
        if key == 27 or key == ord("q"):
            break

    cap.release()
    cv2.destroyAllWindows()


def is_url(path: str) -> bool:
    try:
        p = urllib.parse.urlparse(path)
        return p.scheme in ("http", "https") and p.netloc != ""
    except Exception:
        return False


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Fullscreen looping video player")
    parser.add_argument(
        "video_path",
        nargs="?",
        default=None,
        help=(
            "Path or URL to the video file to play (default: thugmain.mp4 in the executable "
            "directory). If a URL is provided, the file will be downloaded to the temp folder."
        ),
    )
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    requested = args.video_path or resolve_default_video_path()

    # If the argument is a URL, download it to the temp dir
    if is_url(requested):
        tmp = tempfile.gettempdir()
        filename = os.path.join(tmp, "funnyexe_remote_video.mp4")
        try:
            print("Downloading video to", filename)
            download_file(requested, filename)
            video_path = filename
        except Exception as e:
            print("Download failed:", e)
            sys.exit(1)
    else:
        video_path = requested

    display_video_loop(video_path)
