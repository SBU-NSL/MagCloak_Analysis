int color_uvsB = 1;


/*==========Permeability Plotting Macro===========
 * Project: A Compact Magnetic Field Cloaking Device
 * Author: Thomas Krahulik, Sean Jeffas
 * Date Updated: February 12, 2017
 * Purpose: To plot relative magnetic permeability
 *          of ferromagnets as a function of
 *          external magnetic field
 * To run macro: root -l makePlot_uvB.C
 =================================================*/
/*
 * Quantify Uncertainty
 */
double calc_ratio(const char* f_inner, const char* f_outer)
{
  TTree *TDiIn = new TTree();
  TDiIn->ReadFile(f_inner, "di/D");
  TH1D *h_din = new TH1D("h_din", "", 10000, 0, 100);
  TDiIn->Draw("di>>h_din", "", "goff");
  Double_t d_inner = h_din->GetMean(1);
  //  Double_t sig_inner = h_din->GetMeanError(1);
  //  cout << "Mean Inner Diameter: " << d_inner  << " +/- " << sig_inner << endl;

  //  cout<< "Reading Outer Diameters..." << f_outer <<endl;
  TTree *TDiOut = new TTree();
  TDiOut->ReadFile(f_outer, "do/D");
  TH1D *h_dout = new TH1D("h_dout", "", 10000, 0, 100);
  TDiOut->Draw("do>>h_dout", "", "goff");
  Double_t d_outer = h_dout->GetMean(1);
  //  Double_t sig_outer = h_dout->GetMeanError(1);
  //  cout << "Mean Outer Diameter: " << d_outer << " +/- " << sig_outer << endl;
 
  double Ratio = d_inner/d_outer;
  h_din->Delete();
  h_dout->Delete();
  return Ratio;
}


double calc_rsig(const char* f_inner, const char* f_outer)
{
  TTree *TDiIn = new TTree();
  TDiIn->ReadFile(f_inner, "di/D");
  TH1D *h_din = new TH1D("h_din", "", 10000, 0, 100);
  TDiIn->Draw("di>>h_din", "", "goff");
  Double_t d_inner = h_din->GetMean(1);
  Double_t sig_inner = h_din->GetMeanError(1);
  //  cout << "Mean Inner Diameter: " << d_inner  << " +/- " << sig_inner << endl;

  //  cout<< "Reading Outer Diameters..." << f_outer <<endl;
  TTree *TDiOut = new TTree();
  TDiOut->ReadFile(f_outer, "do/D");
  TH1D *h_dout = new TH1D("h_dout", "", 10000, 0, 100);
  TDiOut->Draw("do>>h_dout", "", "goff");
  Double_t d_outer = h_dout->GetMean(1);
  Double_t sig_outer = h_dout->GetMeanError(1);
  //  cout << "Mean Outer Diameter: " << d_outer << " +/- " << sig_outer << endl;

  double Ratio = d_inner/d_outer;
  double Sigma = Ratio*sqrt( pow((sig_inner/d_inner),2) + pow((sig_outer/d_outer),2)  );
  //  cout << Sigma << endl;
  h_din->Delete();
  h_dout->Delete();

  return Sigma;
}

/* =====================================
 * Calibration Measurements and Analysis
   ===================================== */
/*
 * The Calibration function reads in the
 * calibration file and returns a graph
 * that is used to find the relation
 * between current and magentic field
 * for the Helmholtz coil.
 */
TF1* calibrate(
		    const char* f_calib
		    )
{
  /*Read in Calibration File*/
  cout<< "processing file " << f_calib <<endl;
  TTree *TCalib = new TTree();
  TCalib->ReadFile(f_calib, "t/D:I:B");
  int n = TCalib->Draw("I:TMath::Abs(B)", "", "goff");
  TGraph *g_calib = new TGraph(n, TCalib->GetV1(), TCalib->GetV2() );
  g_calib->SetTitle("");
  // g_calib->Draw("AP");
  g_calib->Fit("pol1", "q");
  TF1* calib_fit = g_calib->GetFunction("pol1");

  return calib_fit;

}

/* =====================================
 *      Ferromagnet Scans Analysis
   ===================================== */
/*
 * The Ferromagnet_Scan function reads in
 * a data file of magnetic field scans
 * from within the ferromagnet and returns
 * a graph of the magnetic permeability
 * of the ferromagnet vs the external field
 * provided by the Helmholtz coil.
 */
TGraphErrors* plot_uvB(const TString scan_file,TF1* calib_fit,double R,double R_sig,double &err_val)
{

  /*Read in Data File to ROOT Tree*/
  cout<< "processing file " << scan_file <<endl;
  TTree *TData = new TTree();
  TData->ReadFile(scan_file, "t/D:I:B");
  /*Use TTree Draw command to write branches to usable vector*/
  int n = TData->Draw("I:TMath::Abs(B):0.0:0.005", "", "goff");

  int m=0;
  double ext_test, int_test;
  vector<double> B_ext, B_in, Bratio, u, u_err;
  for(int i = 0; i < n; i++)
    {

      ext_test=calib_fit->Eval(TData->GetV1()[i]);
      int_test=TData->GetV2()[i];

      if(int_test<ext_test){     
	  B_ext.push_back( ext_test );
	  B_in.push_back( int_test );
	  
	  Bratio.push_back( B_in[m] / B_ext[m]);
	  // u.push_back( (Bratio[i]*(R**2) + Bratio[i] - 2 - 2*sqrt((Bratio[i]**2)*(R**2) - Bratio[i]*(R**2) - Bratio[i] + 1 ) ) / (Bratio[i]*(R**2) - Bratio[i]) );
	  u.push_back( (Bratio[m]*(TMath::Power(R, 2.0)) + Bratio[m] - 2 - 2*sqrt((TMath::Power(Bratio[m],2))*(TMath::Power(R, 2.0)) - Bratio[m]*(TMath::Power(R, 2.0)) - Bratio[m] + 1 ) ) / (Bratio[m]*(TMath::Power(R,2.0)) - Bratio[m]) );
	  //u_err.push_back(0.0);
	  u_err.push_back( u[m]*pow(5.5*(pow(0.005/Bratio[m],2)) + 10*(pow(R_sig/R,2)),0.5) );
	  // u_err.push_back( u[i]*( 5.5*((0.01/Bratio[i])**2.))  )**0.5 );
	  m++;
      }
    }

  TGraphErrors *g_uvB = new TGraphErrors(m, &B_ext[0], &u[0], TData->GetV3(), &u_err[0]);

  err_val=u_err[m-1];
  return g_uvB;
}


double convert_R(TString th_file,TString do_file){

  TTree *TDiIn = new TTree();
  TDiIn->ReadFile(th_file, "di/D");
  TH1D *h_din = new TH1D("h_din", "", 10000, 0, 100);
  TDiIn->Draw("di>>h_din", "", "goff");
  Double_t thickness = h_din->GetMean(1);
  //  Double_t sig_inner = h_din->GetMeanError(1);
  //  cout << "Mean Inner Diameter: " << d_inner  << " +/- " << sig_inner << endl;

  //  cout<< "Reading Outer Diameters..." << f_outer <<endl;
  TTree *TDiOut = new TTree();
  TDiOut->ReadFile(do_file, "do/D");
  TH1D *h_dout = new TH1D("h_dout", "", 10000, 0, 100);
  TDiOut->Draw("do>>h_dout", "", "goff");
  Double_t d_outer = h_dout->GetMean(1);
  //  Double_t sig_outer = h_dout->GetMeanError(1);
  //  cout << "Mean Outer Diameter: " << d_outer << " +/- " << sig_outer << endl;
  double d_inner = d_outer-2*thickness;
  double Ratio = d_inner/d_outer;
  h_din->Delete();
  h_dout->Delete();
  return Ratio;
}


double convert_sig(TString th_file, TString do_file)
{
  TTree *TDiIn = new TTree();
  TDiIn->ReadFile(th_file, "di/D");
  TH1D *h_din = new TH1D("h_din", "", 10000, 0, 100);
  TDiIn->Draw("di>>h_din", "", "goff");
  Double_t thickness = h_din->GetMean(1);
  Double_t sig_thickness = h_din->GetMeanError(1);
  //  cout << "Mean Inner Diameter: " << d_inner  << " +/- " << sig_inner << endl;

  //  cout<< "Reading Outer Diameters..." << f_outer <<endl;
  TTree *TDiOut = new TTree();
  TDiOut->ReadFile(do_file, "do/D");
  TH1D *h_dout = new TH1D("h_dout", "", 10000, 0, 100);
  TDiOut->Draw("do>>h_dout", "", "goff");
  Double_t d_outer = h_dout->GetMean(1);
  Double_t sig_outer = h_dout->GetMeanError(1);
  //  cout << "Mean Outer Diameter: " << d_outer << " +/- " << sig_outer << endl;
  double d_inner = d_outer-2*thickness;
  double sig_inner = sqrt(pow(sig_outer,2)+pow(2*sig_thickness,2));
  double Ratio = d_inner/d_outer;
  double Sigma = Ratio*sqrt( pow((sig_inner/d_inner),2) + pow((sig_outer/d_outer),2)  );
  //  cout << Sigma << endl;
  h_din->Delete();
  h_dout->Delete();

  return Sigma;
}



/* ====================================
 * Plot Magnetic Permeability vs Field
   ====================================

syntax: makePlot_uvB( calibration file name, fm scan file name, name to add to legend, inner diameter or thickness file depending on boolean, outer diameter file,
                     boolean with value true if the thickness is measured instead of inner diameter, Legend, permeability value, permeability error)
*/
void makePlot_uvB(TString calibration,TString fmscan,TString name,TString di_file,TString do_file,bool inches,TLegend &leg_uvB,vector<double> &u_val,vector<double> &u_err)
{

  TString file_path = "/home/sean/Dropbox/Stony Brook Research Team Folder/LabVIEW/DATA_Gaussmeter/";

  //// File path may need to be changed if using cryo diameters ////
  TString dia_file_path = "/home/sean/Dropbox/Stony Brook Research Team Folder/MagneticFieldCloakingDevice/ferromagnet_diameters_roomtemp/";

  
 

  vector<double> u_room, f_room, uerr_room, ferr_room;
  vector<double> u_cryo, f_cryo, uerr_cryo, ferr_cryo;
  double err_val;


   /*
       * Permeability Plot for Ferromagnet
       */
      // const TString fmscan_fm104 = "data_perm/DataFile_170210_221924.txt";
      TF1 *calfit_104 = calibrate(file_path+calibration);
      /*Calculate Radius Ratio and Uncertainty*/
      double R_fm104;
      double Rsig_fm104;
      // Check if ferromagnet scan uses thickness instead of inner diameter //
      if (inches){
	R_fm104 = convert_R(dia_file_path+di_file,dia_file_path+do_file);
	Rsig_fm104 = convert_sig(dia_file_path+di_file,dia_file_path+do_file);
      } else {
      R_fm104 = calc_ratio(dia_file_path+di_file, dia_file_path+do_file);
      Rsig_fm104 = calc_rsig(dia_file_path+di_file, dia_file_path+do_file);
      }
      /*Plot u vs B for FM*/
      TGraphErrors *g_fm104 = plot_uvB(file_path+fmscan, calfit_104, R_fm104, Rsig_fm104,err_val);
      g_fm104->Draw("SAME");
      
      if (color_uvsB == 5) color_uvsB++;
      while (color_uvsB > 9 && color_uvsB < 20) {
	color_uvsB++;
      }
      if (color_uvsB >20) color_uvsB=color_uvsB+4;
      
      g_fm104->SetMarkerColor(color_uvsB);
      g_fm104->SetLineColor(color_uvsB);
      color_uvsB++;
      // g_fm104->SetLineColor(kGreen+2);
      // g_fm104->SetMarkerColor(kBlue);
      leg_uvB.AddEntry( g_fm104 , name , "lp");

      u_val.push_back( g_fm104->Eval(40., 0, "") );
      u_err.push_back(err_val );
      
   

  return;
}
