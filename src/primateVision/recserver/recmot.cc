#include "recmot.h"

#define PI 3.14159265

iCub::contrib::primateVision::RecHandleMotionRequest::RecHandleMotionRequest(int period, string *cfg):
  RateThread(period)
{

 Property prop;
 prop.fromConfigFile(cfg->c_str());
 pix2degx    = prop.findGroup("RECMOT").find("PIX2DEGX").asDouble();
 pix2degy    = prop.findGroup("RECMOT").find("PIX2DEGY").asDouble();
 k_vel_verg  = prop.findGroup("RECMOT").find("KVEL_VERG").asDouble();
 k_vel_vers  = prop.findGroup("RECMOT").find("KVEL_VERS").asDouble();
 k_vel_tilt  = prop.findGroup("RECMOT").find("KVEL_TILT").asDouble();
 k_vel_roll  = prop.findGroup("RECMOT").find("KVEL_ROLL").asDouble();
 k_vel_pitch = prop.findGroup("RECMOT").find("KVEL_PITCH").asDouble();
 k_vel_yaw   = prop.findGroup("RECMOT").find("KVEL_YAW").asDouble();
 vor_on      = (bool) prop.findGroup("VOR").find("VOR_ON").asInt();
 vor_k_rol   = prop.findGroup("RECMOT").find("K_ROL").asDouble();
 vor_k_pan   = prop.findGroup("RECMOT").find("K_PAN").asDouble();
 vor_k_tlt   = prop.findGroup("RECMOT").find("K_TLT").asDouble();
 vor_d_rol   = prop.findGroup("RECMOT").find("D_ROL").asDouble();
 vor_d_pan   = prop.findGroup("RECMOT").find("D_PAN").asDouble();
 vor_d_tlt   = prop.findGroup("RECMOT").find("D_TLT").asDouble();
 headfollow  = prop.findGroup("RECMOT").find("HEADFOLLOW").asDouble();


  angles[0]=0.0;
  angles[1]=0.0;
  angles[2]=0.0;
  angles[3]=0.0;
  angles[4]=0.0;
  angles[5]=0.0;
  suspend = 0;
  locked_to = NO_LOCK;

  vor_vels[0]=0.0; 
  vor_vels[1]=0.0; 
  vor_vels[2]=0.0; 
  vor_vels[3]=0.0; 
  vor_vels[4]=0.0; 
  vor_vels[5]=0.0; 

  gyro_vel[0] = 0.0;
  gyro_vel[1] = 0.0;
  gyro_vel[2] = 0.0;

}




void iCub::contrib::primateVision::RecHandleMotionRequest::run(){
  
  
  if (motion){

    if (suspend>0){
      suspend--;
      printf("RecServer: Suspended %d.\n",suspend);
    }
    
    //incorporate motion requests:
    rmq = inPort_mot->read(false);
    //skip updating desired target pos for "suspend" * period:
    if (rmq!=NULL && suspend==0){
      
      if (locked_to!=NO_LOCK){
	printf("RecServer: Locked to: %d\n",locked_to);
      }

      if (locked_to == rmq->content().lockto || locked_to == NO_LOCK){
	//accept command..
	//head pitch (angles[0]) and yaw (angles[2]) done automatically now!!                
	angles[0] = 0.0;
	angles[1] = rmq->content().deg_r; 
	angles[2] = 0.0;                            
	angles[3] = -rmq->content().pix_y*pix2degy;      
	//angles[4] = pix2degx * (rmq->content().pix_xr+rmq->content().pix_xl); //version aprox: (l+r)/2 better: atan( (tan(L)+tan(R))/2 )
    angles[4] = (180.0/PI) * atan(  (tan((PI/180.0)*pix2degx*rmq->content().pix_xl) + tan((PI/180.0)*pix2degx*rmq->content().pix_xr))/2.0 );
	angles[5] = pix2degx*(rmq->content().pix_xl-rmq->content().pix_xr); //vergence  =  l - r
	//printf("RecServer: Motion command received: (%f,%f,%f)\n",rmq->content().pix_xl,rmq->content().pix_xr,rmq->content().pix_y);
	printf("RecServer: Motion command forwarded: (%f,%f,%f)\n",angles[3],angles[4],angles[5]);
	relative  = rmq->content().relative;
	suspend   = rmq->content().suspend;
	locked_to = rmq->content().lockto;
	if (rmq->content().unlock == 1){
	  locked_to = NO_LOCK;
	}
	
	if (relative){//relative move requested:
	  //convert to absolute:
	  for (int i=0;i<6;i++){
	    angles[i] += enc[i];
	  }
	}
	if(angles[5]<0.0){
	  angles[5]=0.0;
	}
	
    //incorporate HEAD FOLLOWING EYES:
    if (headfollow != 0.0){
      //sub a bit from version (angles[4]), add it to yaw (angles[2]):
	  angles[4] -= angles[4]*headfollow; //head does % of motion according to 'headfollow'
      angles[2] -= angles[4]*headfollow;
      //sub from eye tilt (angles[3]), add to head tilt (angles[0]):
	  angles[3] -= angles[3]*headfollow;
      angles[0] += angles[3]*headfollow;
     }


      }
    }
    
    






    //incorporate VOR:
    if (vor_on){

      //get gyro accelleration data:
      gyro = inPort_inertial->read(false); //false = non-blocking
      if (gyro!=NULL){
	
	//printf("RecServer: VOR Data.\n");
	/*inertial signal 12-chan as follows:
	 * (*gyro)[0];//r
	 * (*gyro)[1];//p
	 * (*gyro)[2];//y
	 * 
	 * (*gyro)[3];//ax
	 * (*gyro)[4];//ay
	 * (*gyro)[5];//az
	 * 
	 * (*gyro)[6];//gx
	 * (*gyro)[7];//gy
	 * (*gyro)[8];//gz
	 *
	 * (*gyro)[9]; //mx
	 * (*gyro)[10];//my
	 * (*gyro)[11];//mz
	 */
	
	//integrate to get vel:
	gyro_vel[0] += (*gyro)[6];//gx
	gyro_vel[1] += (*gyro)[7];//gy
	gyro_vel[2] += (*gyro)[8];//gz
	
	//tend towards zero to eliminate residual velocity drift:
	gyro_vel[0] *= vor_d_rol;
	gyro_vel[1] *= vor_d_tlt;
	gyro_vel[2] *= vor_d_pan;
	
	//corrective motion cmd (only vestibular axes)
	vor_vels[0] =  vor_k_rol*(gyro_vel[0]); //roll correction
	vor_vels[1] =  -vor_k_tlt*(gyro_vel[1]); //tilt correction
	vor_vels[2] =  vor_k_pan*(gyro_vel[2]); //pan correction
      }
      //else{
      //printf("RecServer: No VOR Data.\n");
      //	vor_vels[0]=0.0;
      //	vor_vels[1]=0.0;
      //	vor_vels[2]=0.0;
      //}
      
    }


    
    //get encoder data:
    if (encs->getEncoders(enc_tmp)){
      for (int i=0;i<6;i++){
	enc[i] = enc_tmp[i];
      }
    }
    
    vels[0] = k_vel_pitch*(angles[0]-enc[0]);
    vels[1] = k_vel_roll*(angles[1]-enc[1]);
    vels[2] = k_vel_yaw*(angles[2]-enc[2]);
    vels[3] = k_vel_tilt*(angles[3]-enc[3]);
    vels[4] = k_vel_vers*(angles[4]-enc[4]);
    vels[5] = k_vel_verg*(angles[5]-enc[5]);
    
    //incorporate VOR corrections:
    if (vor_on){
      vels[1] += vor_vels[0];  //head roll
      vels[3] += vor_vels[1];  //eye tilt
      vels[4] += vor_vels[2];  //eye pan
    }
    
    
    //send!
    //printf("RecServer: Move (%f,%f,%f)\n", vels[3],vels[4],vels[5]);
    vel->velocityMove(vels);

 
  }
  
  
  
    
}



