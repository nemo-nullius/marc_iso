#include "decl.h"
#include "marc_iso_entry.h"

struct KeyInfo
{
    size_t sum;
    set<string> subkeys;
};

struct MarcIsoFile
{
public:
    MarcIsoFile(const string s) : filepath(s),
                                  err_records_filename(s.substr(0, s.rfind('.')) + "_err_rec.txt"),
                                  bad_records_filename(s.substr(0, s.rfind('.')) + "_bad_rec.txt"),
                                  records_csv_filename(s.substr(0, s.rfind('.')) + "_csv.txt"){};
    void work();
    size_t bad_records_sum() { return bad_records.size(); };
    size_t err_records_sum() { return err_records.size(); };
    size_t records_processed_sum() { return records_proc.size(); };
    size_t records_original_sum() { return records_raw.size(); };
    string filepath; // records_orig_filename
    string err_records_filename;
    string bad_records_filename;
    string records_csv_filename;

private:
    // TODO: clear of some variants
    // if this class needs to be reloaded
    ifstream infile;
    vector<string> records_raw;
    vector<size_t> bad_records;
    vector<size_t> err_records;
    vector<mmm> records_proc;
    map<string, KeyInfo> keys_templ;
    string records_csv;
    void analyse();        // infile, records_raw, err_records, bad_records, records_proc,
    void get_keys_templ(); // keys_templ
    void records2csv();    // records_csv
    void save_rec_raw(const string filename, const vector<size_t> &outvec)
    {
        cout << filename << endl;
        //clog << outvec.size() << endl;
        ofstream outfile(filename, ios::out | ios::binary);
        for (auto &i : outvec)
        {
            //clog << records_raw[i] << endl;
            outfile << records_raw[i] << "\n";
        }
        outfile.close();
    };
    void save_str(const string filename, const string &content)
    {
        cout << filename << endl;
        ofstream outfile(filename, ios::out | ios::binary);
        outfile << content;
        outfile.close();
    }
    void save_bad_records() { save_rec_raw(bad_records_filename, bad_records); }
    void save_err_records() { save_rec_raw(err_records_filename, err_records); }
    void save_records_csv() { save_str(records_csv_filename, records_csv); }
    void str_addn(string &s, const string to_be_added, size_t n = 1)
    {
        for (size_t i = 0; i < n; ++i)
            s += to_be_added;
    }
};

inline void MarcIsoFile::analyse()
{
    // get all records from file
    infile.open(filepath);
    for (string line; getline(infile, line);)
    {
        // linux file end best
        line = (line[line.size() - 1] == '\x0d') ? string(line.begin(), line.end() - 1) : line; // \x0d == \r, \x0a == \n. getline will delete \n
        records_raw.push_back(line);
    }
    for (size_t i = 0; i != records_raw.size(); ++i)
    {
        auto &rec = records_raw[i];
        MarcIsoEntry iso_rec(rec);
        auto fields = iso_rec.get_fields();
        if (iso_rec.is_err_data()) // will not insert this record
        {
            err_records.push_back(i);
        }
        else // will push
        {
            if (iso_rec.is_bad_data())
                bad_records.push_back(i);
            records_proc.push_back(fields);
        }
    }
}

inline void MarcIsoFile::get_keys_templ() // keys_templ
{
    for (const auto &rec_proc : records_proc) // vector<...>
    {
        map<string, size_t> keys_sum_one_record; // to check duplicate keys in one record
        for (const auto &fkv : rec_proc)         // pair<...>
        {
            const auto &fk = fkv.first;
            const auto &fv = fkv.second;

            // update keys_templ
            if (keys_templ.find(fk) == keys_templ.end()) // field key not exist - add it
            {
                keys_templ[fk] = KeyInfo{};
            }
            for (const auto &sfkv : fv) // update all subkeys (in a set)
                keys_templ[fk].subkeys.insert(sfkv.first);

            // update keys_sum_one_record
            if (keys_sum_one_record.find(fk) == keys_sum_one_record.end())
                keys_sum_one_record[fk] = 1;
            else
                ++keys_sum_one_record[fk];

            // update subkeys in keys_templ - choose the bigger one
            for (const auto &k1rec : keys_sum_one_record)
            {
                const auto &key_name = k1rec.first;
                const auto &key_sum = k1rec.second;
                keys_templ[key_name].sum = max(keys_templ[key_name].sum, key_sum);
            }
        }
    }
}

inline void MarcIsoFile::records2csv() // records_csv - LF
{
    size_t lnlen = 0; // length of one line - row length
    // make table head - spread keys_templ
    for (const auto &key_templ : keys_templ)
    {
        const auto &key_name = key_templ.first;
        const auto &key_sum = key_templ.second.sum;
        const auto &subkeys = key_templ.second.subkeys;
        for (size_t i = 0; i < key_sum; ++i)
        {
            string suffix = (key_sum < 2) ? string("") : "@" + std::to_string(i);
            for (auto &sk : subkeys)
            {
                str_addn(records_csv, key_name + ":" + sk + suffix + "\t", 1);
                lnlen += 1;
            }
        }
    }
    records_csv += "\n";

    // make table body
    for (const auto &rec_kv : records_proc) // convert each record one by one
    {
        for (const auto &key_templ : keys_templ) // each key in keys_templ
        {
            const auto &key_name = key_templ.first;
            const auto &key_sum = key_templ.second.sum;
            const auto &subkeys = key_templ.second.subkeys;
            auto key_name_pos_range = rec_kv.equal_range(key_name);
            /*
            if (key_name_pos_range.second == key_name_pos_range.first)
            { // not found - add "\t" & continue to next key
                str_addn(records_csv, "\t", subkeys.size() * key_sum);
                continue;
            }
            */
            size_t key_sum_one_record = 0; // how many instances of this key (like 701) are there in this record
            for (auto pos = key_name_pos_range; pos.first != pos.second; ++pos.first)
            {
                const auto &rec_skv = pos.first->second;
                for (auto &sk : subkeys)
                {
                    if (rec_skv.find(sk) == rec_skv.end()) // subkey not found
                    {
                        str_addn(records_csv, "\t", 1);
                    }
                    else
                    {
                        str_addn(records_csv, rec_skv.at(sk) + "\t"); // use at() method: this is a const map.
                    }
                }
                ++key_sum_one_record;
            }
            // dump blank elements - if key not found, key_sum_one_record == 0
            str_addn(records_csv, "\t", subkeys.size() * (key_sum - key_sum_one_record));
        }
        str_addn(records_csv, "\n", 1);
    }
}

inline void MarcIsoFile::work()
{
    clog << "[INFO] Start analyse." << endl;
    analyse();
    clog << "[INFO] Save bad records." << endl;
    save_bad_records();
    clog << "[INFO] Save err records." << endl;
    save_err_records();
    clog << "[INFO] Convert to csv format." << endl;
    get_keys_templ();
    records2csv();
    clog << "[INFO] Save csv records." << endl;
    save_records_csv();
    clog << "[INFO] All finished!" << endl;
}