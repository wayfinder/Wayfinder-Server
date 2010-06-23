import java.awt.image.ImageObserver;
import java.awt.Graphics;

/**
 *   Represents one square in the MercatorMapComponent
 */
abstract class MercatorMap {

   int x;
   int y;
   int zoom;
   int width;
   int height;

   public MercatorMap(MercatorMapParams params, int width, int height) {
      x = params.getX();
      y = params.getY();
      zoom = params.getZoom();
      this.width = width;
      this.height = height;
   }

   public abstract void load(byte[] data);

   public abstract void draw(Graphics g, ImageObserver obs, int left_world_x,
         int top_world_y);

}
