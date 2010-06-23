class IsabRouteElement {

   protected double latitude;
   protected double longitude;
   protected double heading;
   protected double speed;
   protected double knots;

   public IsabRouteElement(double lat, double lon, double heading, double speed) {
      this.latitude = lat;
      this.longitude = lon;
      this.heading = heading;
      this.speed = speed;
      this.knots = 1.9438445 * speed;
   }

   public double getLat() {
      return latitude;
   }

   public double getLon() {
      return longitude;
   }

   public double getSpeed() {
      return knots;
   }

   public double getMpsSpeed() {
      return speed;
   }

   public double getBearing() {
      return heading;
   }

}
