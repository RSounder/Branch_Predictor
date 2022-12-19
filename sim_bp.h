#ifndef SIM_BP_H
#define SIM_BP_H

typedef struct bp_params{
    unsigned long int K;
    unsigned long int M1;
    unsigned long int M2;
    unsigned long int N;
    char*             bp_name;
}bp_params;

class genericBimodal {
public:
  vector<int> bimodalVec;
  unsigned long num_rows;
  unsigned long misPredictions;
  unsigned long totalPredictions;

  genericBimodal(unsigned long m_num = 0) {
    num_rows = pow(2,m_num); 
    misPredictions = 0;
    totalPredictions = 0;
    bimodalVec = vector<int>(num_rows, 2);
  }

  unsigned long convertBinStr2Ulong(string str1) {
    unsigned long val = 0;
    unsigned long int i = 0;

    while (str1[i] == '0' || str1[i] == '1') {
      // Found another digit.
      val <<= 1;
      val += str1[i] - '0';
      i++;
    }
    return val;
  }

  unsigned long getIndexFromAddress(unsigned long int addr) {
      /* getIndexFromAddress takes a unsigned long int address and returns index based on num_rows
      how? 
      convert addr to binary string; discard last two lsb bits
      number of index bits = log2(num_rows)
      get string from 30 - num_index_bits to end */

      string addrStr = (bitset<32>(addr)).to_string();
      addrStr.resize(30); //removing two lsb vals
      string index = addrStr.substr((30 - log2(num_rows)));

      return convertBinStr2Ulong(index); //return unsigned long
  }

    void incDecCount(unsigned long index, string selector) {
        totalPredictions++;
        if(selector == "inc" && bimodalVec[index] < 3) {
            if(bimodalVec[index] < 2) {
                misPredictions++;
            }
            bimodalVec[index] = bimodalVec[index] + 1;
        } else if(selector == "dec" && bimodalVec[index] > 0) {
            if(bimodalVec[index] > 1) {
                misPredictions++;
            }
            bimodalVec[index] = bimodalVec[index] - 1;
          }
    }
  
    int returnPrediction(unsigned long addr) {
        int temp = bimodalVec[getIndexFromAddress(addr)];
        return temp;
    }

  void printAll(string sel = "") {
  /*
  OUTPUT
    number of predictions:    2000000
    number of mispredictions: 623425
    misprediction rate:       31.17%
    FINAL BIMODAL CONTENTS
  */
    if(sel != "onlyTableVals") { //init vals is used for testing init values of prediction tables. 
        cout<<"OUTPUT"<<endl
            <<"number of predictions:\t"<<totalPredictions<<endl
            <<"number of mispredictions:\t"<<misPredictions<<endl
            <<"misprediction rate:\t"; printf("%.2f%%\n",(double(misPredictions)/double(totalPredictions)) * 100);
        cout<<"FINAL BIMODAL CONTENTS"<<endl;
    }

    if (sel != "onlyPredVals") {
        for (unsigned long i = 0; i < num_rows; i++) {
          cout<<i<<" "<<bimodalVec[i]<<endl;
        }
    }
  }

};

class genericGshare {
    public:
        string globalBranchHistory;

        vector<int> gshareVec;
        unsigned long num_rows;
        unsigned long num_gbh_bits;

        unsigned long misPredictions;
        unsigned long totalPredictions;

        genericGshare(unsigned long m_num = 0, unsigned long n_num = 0) {
            num_rows = pow(2,m_num);
            num_gbh_bits = n_num;
            globalBranchHistory = "";
            for (unsigned long i = 0; i < n_num; i++) {
                globalBranchHistory = globalBranchHistory + "0"; 
            }

            misPredictions = 0;
            totalPredictions = 0;
            gshareVec = vector<int>(num_rows, 2);
        }

        string getIndexFromAddress(unsigned long int addr) {
            /* getIndexFromAddress takes a unsigned long int address and returns index based on num_rows
            how? 
            convert addr to binary string; discard last two lsb bits
            number of index bits = log2(num_rows)
            get string from 30 - num_index_bits to end */

            string addrStr = (bitset<32>(addr)).to_string();
            addrStr.resize(30); //removing two lsb vals
            string index = addrStr.substr((30 - log2(num_rows)));

            return index; //return unsigned long
        }

        unsigned long convertBinStr2Ulong(string str1) {
            unsigned long val = 0;
            unsigned long int i = 0;

            while (str1[i] == '0' || str1[i] == '1') {
              // Found another digit.
              val <<= 1;
              val += str1[i] - '0';
              i++;
            }
            return val;
        }

        void updateGbh(string str) {
            //we have to insert str in msb position and delete lsb
            globalBranchHistory = str + globalBranchHistory;
            globalBranchHistory.resize(num_gbh_bits);
        }

        unsigned long getIndexToBranchHistory(unsigned long int addr) {
            //convert addr and gbh to ul and XOR
            string indStr = getIndexFromAddress(addr);
            string nBit_msb_of_index = indStr.substr(0, num_gbh_bits);
            string rest_of_index = indStr.substr(num_gbh_bits);

            string xorOut = (bitset<32>(convertBinStr2Ulong(globalBranchHistory) ^ convertBinStr2Ulong(nBit_msb_of_index))).to_string();
            return (convertBinStr2Ulong( xorOut + rest_of_index ));
        }

        void incDecCount(unsigned long index, string selector, bool gbhUpdate = true) {
            totalPredictions++;
            if(selector == "inc") {
                if(gshareVec[index] <= 1) {
                    misPredictions++;
                }
                if(gshareVec[index] != 3) {
                    gshareVec[index] = gshareVec[index] + 1;
                }
                if(gbhUpdate) {
                    updateGbh("1"); //update gbh with actual outcome
                }
            } else if(selector == "dec") {
                if(gshareVec[index] >= 2) {
                    misPredictions++;
                }
                if(gshareVec[index] != 0) {
                    gshareVec[index] = gshareVec[index] - 1;
                }
                if(gbhUpdate) {
                    updateGbh("0"); //update gbh with actual outcome
                }
                
            }
        }
        
        int returnPrediction(unsigned long int addr) {
            int temp = gshareVec[getIndexToBranchHistory(addr)];
            return temp;
        }

        void printAll(string sel = "") {
            /*
            OUTPUT
             number of predictions:    2000000
             number of mispredictions: 433345
             misprediction rate:       21.67%
            FINAL GSHARE CONTENTS
            */
            if(sel != "onlyTableVals") { //init vals is used for testing init values of prediction tables.  
                cout<<"OUTPUT"<<endl
                    <<"number of predictions:\t"<<totalPredictions<<endl
                    <<"number of mispredictions:\t"<<misPredictions<<endl
                    <<"misprediction rate:\t"; printf("%.2f%%\n",(double(misPredictions)/double(totalPredictions)) * 100);
                cout<<"FINAL GSHARE CONTENTS"<<endl;                
            }
            if(sel != "onlyPredVals") {
                for (unsigned long i = 0; i < num_rows; i++) {
                  cout<<i<<" "<<gshareVec[i]<<endl;
                }
            }

        }
};

class genericHybrid {

    public:
        //data declaration

        genericBimodal bimodal;
        genericGshare gshare;

        vector<int> chooserVec;
        unsigned long k, m1, n, m2, num_chooser_rows;
        genericHybrid(unsigned long k_num = 0, unsigned long m1_num = 0 , unsigned long n_num = 0, unsigned long m2_num = 0) {
            k = k_num;
            m1 = m1_num;
            n = n_num;
            m2 = m2_num;
            
            num_chooser_rows = pow(2,k);
            
            chooserVec = vector<int>(num_chooser_rows,1); //vector initialising num_chooser_rows rows with value 1
            
            bimodal = genericBimodal(m2);
            gshare = genericGshare(m1, n);
            
        }

        unsigned long convertBinStr2Ulong(string str1) {
            unsigned long val = 0;
            unsigned long int i = 0;

            while (str1[i] == '0' || str1[i] == '1') {
              // Found another digit.
              val <<= 1;
              val += str1[i] - '0';
              i++;
            }
            return val;
        }
        
        unsigned long getIndexFromAddress(unsigned long int addr) {
            /* getIndexFromAddress takes a unsigned long int address and returns index based on num_rows
            how? 
            convert addr to binary string; discard last two lsb bits
            get string from k+1 bit to end */

            string addrStr = (bitset<32>(addr)).to_string();
            addrStr.resize(30); //removing two lsb vals
            string index = addrStr.substr(30 - k);
            return convertBinStr2Ulong(index); //return unsigned long
        }
        
        void choosePredictions(unsigned long int addr, string actualOutcome) {
            //get predictions from both predictors
            int bimodalPred = bimodal.returnPrediction(addr);
            int gsharePred = gshare.returnPrediction(addr);
            unsigned long chooserIndex = getIndexFromAddress(addr); 
            int chooserPred = chooserVec[chooserIndex];
            /*If the chooser counter value is greater than or equal to 2, then use the prediction that was
                obtained from the gshare predictor, otherwise use the prediction that was obtained from the bimodal predictor.*/
            if(chooserPred >=2) {
                //gshare
                if(actualOutcome == "t") {
                    //taken; inc without gbh
                    gshare.incDecCount(gshare.getIndexToBranchHistory(addr), "inc", false);

                } else {
                    //not taken; dec without gbh
                    gshare.incDecCount(gshare.getIndexToBranchHistory(addr), "dec", false);
                }
            } else {
                //bimodal
                if(actualOutcome == "t") {
                    //taken; inc
                    bimodal.incDecCount(bimodal.getIndexFromAddress(addr), "inc");
                } else {
                    //not taken; dec
                    bimodal.incDecCount(bimodal.getIndexFromAddress(addr), "dec");
                }
            }
            
            //update gbh for every prediction
            if (actualOutcome == "t") { gshare.updateGbh("1"); } else { gshare.updateGbh("0"); }
            
            //update chooser
            if(((actualOutcome == "t") && (bimodalPred >=2 && gsharePred < 2)) || ((actualOutcome == "n") && (bimodalPred <=1 && gsharePred > 1))) {
                //bimodal is correct and gshare is incorrect
                if(chooserVec[chooserIndex] > 0) {
                   chooserVec[chooserIndex] = chooserVec[chooserIndex] - 1;
                }
            } else if (((actualOutcome == "t") && (bimodalPred < 2 && gsharePred >= 2)) || ((actualOutcome == "n") && (bimodalPred > 1 && gsharePred <= 1))) {
                //gshare is correct and bimodal is incorrect
                if(chooserVec[chooserIndex] < 3) {
                    chooserVec[chooserIndex] = chooserVec[chooserIndex] + 1;
                }
            }
        }
        
        void printAll() {
            /*
            OUTPUT
             number of predictions:    2000000
             number of mispredictions: 207922
             misprediction rate:       10.40%
            FINAL CHOOSER CONTENTS
            */

            cout<<"OUTPUT"<<endl
                <<"number of predictions:\t"<< bimodal.totalPredictions + gshare.totalPredictions <<endl
                <<"number of mispredictions:\t"<< bimodal.misPredictions + gshare.misPredictions <<endl
                <<"misprediction rate:\t"; 
            printf("%.2f%%\n",(double(bimodal.misPredictions + gshare.misPredictions)/double(bimodal.totalPredictions + gshare.totalPredictions)) * 100);
            cout<<"FINAL CHOOSER CONTENTS"<<endl;

            for (unsigned long i = 0; i < num_chooser_rows; i++) {
              cout<<i<<" "<<chooserVec[i]<<endl;
            }
            cout<<"FINAL GSHARE CONTENTS"<<endl;
            gshare.printAll("onlyTableVals");
            cout<<"FINAL BIMODAL CONTENTS"<<endl;
            bimodal.printAll("onlyTableVals");
        }
    
};

#endif
