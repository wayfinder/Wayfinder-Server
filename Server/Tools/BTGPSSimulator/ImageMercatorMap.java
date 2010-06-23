import java.awt.Image;
import java.awt.image.ImageObserver;
import java.awt.Graphics;

public class ImageMercatorMap extends MercatorMap {

   private Image image;

   /**
    *   Temporary constructor. Should load map in other thread
    *   with load
    */
   public ImageMercatorMap(MercatorMapParams params, int width, int height) {
      super(params, width, height);
   }

   public void load(byte[] data) {
      if (data != null) {
         image = java.awt.Toolkit.getDefaultToolkit().createImage(data);
      }
   }

   public void draw(Graphics g, ImageObserver obs, int left_world_x,
         int top_world_y) {
      Image im = getImage();
      if (im == null) {
         return;
      }
      int imageX = x - left_world_x;
      int imageY = top_world_y - y - height;
      //System.err.println("(x,y) = (" + imageX + "," + imageY + ")" );
      g.drawImage(im, imageX, imageY, obs);
      //        g.drawLine( x - left_world_x,
      //                    top_world_y - y - height,
      //                    x - left_world_x + width,
      //                    top_world_y - y );

   }

   public synchronized Image setImage(Image im) {
      Image tmp = image;
      image = im;
      return tmp;
   }

   public synchronized Image getImage() {
      return image;
   }

}
