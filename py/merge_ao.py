from PIL import Image


def main():
    # Open the images
    img1 = Image.open(r"C:\Users\woos8\Desktop\pc_dl_av_185b_upper1_a.png")
    img2 = Image.open(r"C:\Users\woos8\Desktop\pc_dl_av_185b_upper1_o.png")

    # Convert img2 to 'L' mode
    img2 = img2.convert('L')

    print(img1.mode)
    print(img2.mode)

    # Use img2 as the alpha channel for img1
    img1.putalpha(img2)

    # Save the image
    img1.save(r"C:\Users\woos8\Desktop\ao_merged.png")


if __name__ == "__main__":
    main()
