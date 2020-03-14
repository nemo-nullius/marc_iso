//#include "marc_iso_entry.h"
#include "decl.h"
#include "marc_iso_file.h"

int main(int argc, char **argv)
{
    //string filename = "./workplace/sample.iso";
    //string filename = "./workplace/sample_bad_error.iso";
    if (argc < 2)
    {
        cout << "Wrong parameter.\n"
                "Usage:"
                "convmarciso.exe [filename]\n";
        return 1;
    }

    string filename = argv[1];
    auto isofilehandler = MarcIsoFile(filename);
    isofilehandler.work();

    auto bad_records_sum = isofilehandler.bad_records_sum();
    auto err_records_sum = isofilehandler.err_records_sum();
    auto records_orig_sum = isofilehandler.records_original_sum();
    auto records_proc_sum = isofilehandler.records_processed_sum();

    cout << endl
         << "Process Statics:" << endl;
    cout << "Original records:\t" << records_orig_sum << "\t[" << isofilehandler.filepath << "]" << endl;
    cout << "Bad records: \t" << bad_records_sum << "\t[" << isofilehandler.bad_records_filename << "]" << endl;
    cout << "Err records: \t" << err_records_sum << "\t[" << isofilehandler.err_records_filename << "]" << endl;
    cout << "Processed records:\t" << records_proc_sum << "\t(Original records - Err records)\t["
         << isofilehandler.records_csv_filename << "]" << endl;
    return 0;
}
