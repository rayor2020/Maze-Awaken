from PIL import Image
import random
import os

# 配置参数
IMAGE_SIZE = 16  # 每张小图的尺寸
GRID_WIDTH = 121  # 横向小图数量
GRID_HEIGHT = 68  # 纵向小图数量
STONE_IMAGES = [f"stone{i}.png" for i in range(14)]  # 9张原始图片文件名


def create_stone_mosaic():
    # 验证原始图片是否存在
    for img_name in STONE_IMAGES:
        if not os.path.exists(img_name):
            print(f"错误: 找不到图片 {img_name}")
            return None

    # 加载所有原始图片
    stone_images = []
    for img_name in STONE_IMAGES:
        try:
            img = Image.open(img_name)
            # 确保图片是16x16尺寸
            if img.size != (IMAGE_SIZE, IMAGE_SIZE):
                img = img.resize((IMAGE_SIZE, IMAGE_SIZE))
            stone_images.append(img)
        except Exception as e:
            print(f"加载图片 {img_name} 时出错: {e}")
            return None

    # 创建新的大图
    mosaic_width = GRID_WIDTH * IMAGE_SIZE
    mosaic_height = GRID_HEIGHT * IMAGE_SIZE

    # 确定输出图片的模式（使用第一张图片的模式）
    output_mode = stone_images[0].mode
    # 如果是透明背景，使用RGBA模式
    if output_mode == "P" or "transparency" in stone_images[0].info:
        output_mode = "RGBA"

    mosaic = Image.new(output_mode, (mosaic_width, mosaic_height))

    # 随机选择并拼接图片
    print(f"正在生成 {GRID_WIDTH}x{GRID_HEIGHT} 的马赛克图...")

    for y in range(GRID_HEIGHT):
        for x in range(GRID_WIDTH):
            # 随机选择一张原始图片
            random_img = random.choice(stone_images)
            # 计算粘贴位置
            paste_x = x * IMAGE_SIZE
            paste_y = y * IMAGE_SIZE
            # 粘贴到对应位置
            mosaic.paste(random_img, (paste_x, paste_y))

        # 显示进度
        if (y + 1) % 5 == 0 or y == GRID_HEIGHT - 1:
            print(f"进度: {y + 1}/{GRID_HEIGHT} 行")

    return mosaic


def main():
    print("开始创建石头马赛克图...")
    print(f"原始图片: {', '.join(STONE_IMAGES)}")
    print(f"输出尺寸: {GRID_WIDTH * IMAGE_SIZE} x {GRID_HEIGHT * IMAGE_SIZE} 像素")
    print(f"小图数量: {GRID_WIDTH} x {GRID_HEIGHT} = {GRID_WIDTH * GRID_HEIGHT} 个")

    mosaic = create_stone_mosaic()

    if mosaic:
        # 保存结果
        output_filename = "stone_mosaic.png"
        mosaic.save(output_filename)
        print(f"\n马赛克图已保存为: {output_filename}")

        # 显示图片（可选）
        try:
            mosaic.show()
        except:
            print("注意: 无法自动显示图片，请手动打开查看")

        return output_filename
    else:
        print("创建马赛克图失败")
        return None


if __name__ == "__main__":
    main()
