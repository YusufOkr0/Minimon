# MiniMon Sistem İzleyici

MiniMon, C dilinde yazılmış bir komut satırı sistem izleme aracıdır. CPU, bellek, disk, ağ, işlemler gibi çeşitli sistem kaynakları hakkında bilgi sağlar ve arka planda raporlama yapma özelliği içerir.

## Uyumluluk

Bu uygulama, Linux sistemlerine özgü çekirdeğe ait dosyalara (`/proc` gibi) ve araçlara dayanır. Bu nedenle **yalnızca Linux ortamlarında çalışır.**

Windows kullanıcısıysanız, uygulamayı çalıştırmak için **Windows Subsystem for Linux (WSL)** kullanmanız zorunludur. Uygulamayı derleme ve çalıştırma adımlarını WSL ortamındaki bir Linux terminalinde gerçekleştireceksiniz.

## Windows Kullanıcısı İçin Kurulum ve Çalıştırma Adımları (WSL Üzerinden)

Bu adımlar, WSL'nin sisteminizde kurulu olduğunu ve bir Linux kullanıcı adı/şifresi belirlediğinizi varsayar.

1.  **Proje Dosyalarını İndirin:**
    *   WSL terminalini açın.
    *   Proje dosyalarını GitHub'dan indirmek için `git clone` komutunu kullanın. Kendi ev dizininiz (`~` veya `/home/sizin_kullanici_adiniz`) altında indirmeniz en temiz yoldur:
        ```bash
        git clone https://github.com/YusufOkr0/Minimon.git
        cd Minimon # İndirilen projenin klasörüne girin
        ```
    *   *(Eğer dosyaları Windows tarafından kopyaladıysanız, WSL terminalinde o dosyalara gidip (örneğin, `cd /mnt/c/Users/KullaniciAdiniz/Desktop/Minimon`) işlemlere devam edebilirsiniz, ancak WSL ev dizinine kopyalamak genellikle daha sorunsuz olabilir: `cp -r /mnt/c/Yol/Minimon ~/ && cd ~/Minimon`)*

2.  **Gerekli Araçları Yükleyin:**
    *   Uygulamayı derlemek için C derleyicisine (GCC) ve `make` aracına ihtiyacınız var. WSL terminalinde bu araçları yükleyin:
        ```bash
        sudo apt update         # Paket listesini günceller
        sudo apt install build-essential # GCC, make ve diğer gerekli araçları yükler
        ```
        *(Bu komutlar için root yetkisi gerekir, `sudo` sizden WSL Linux kullanıcı şifrenizi isteyecektir.)*

3.  **Uygulamayı Derleyin:**
    *   Hala proje dosyalarının bulunduğu dizinde olduğunuzdan emin olun (adım 1'de `cd Minimon` komutuyla girdiğiniz yer).
    *   Uygulamayı derlemek için `make` komutunu çalıştırın:
        ```bash
        make
        ```
    *   Derleme başarılı olursa, aynı dizin içinde `minimon` adında çalıştırılabilir bir dosya oluşacaktır.

4.  **Uygulamayı Çalıştırın (Root Yetkisi Gerekli):**
    *   MiniMon, sistemin derinliklerindeki bilgilere erişmek için **root (yönetici) yetkisine ihtiyaç duyar.** Uygulamanın tüm özelliklerinden tam olarak yararlanmak ve herhangi bir izin hatasıyla karşılaşmamak için, **her çalıştırdığınızda komutun başına `sudo` eklemelisiniz.**
    *   WSL terminalinde `minimon` çalıştırılabilir dosyasını komutunuzla birlikte, `sudo` kullanarak çalıştırın:
        ```bash
        sudo ./minimon cpu         # CPU kullanımını gösterir
        sudo ./minimon mem         # Bellek kullanımını gösterir
        sudo ./minimon disk        # Disk G/Ç bilgilerini gösterir
        sudo ./minimon net         # Ağ arayüzlerini listeler
        sudo ./minimon connect     # Aktif bağlantıları gösterir (özellikle root gerektirir)
        sudo ./minimon proc        # Çalışan tüm işlemleri listeler
        sudo ./minimon top 5       # En çok CPU kullanan 5 işlemi gösterir
        sudo ./minimon report      # Arka planda raporlamayı başlatır (özellikle root gerektirir)
        sudo ./minimon stop-report # Arka planda raporlamayı durdurur (özellikle root gerektirir)
        sudo ./minimon --help      # Yardım menüsünü gösterir
        ```
    *   `sudo` kullandığınızda, WSL sizden Linux kullanıcı şifrenizi girmenizi isteyecektir. Şifrenizi girdikten sonra komut root yetkisiyle çalışacaktır.

## Özellikler

*   **CPU:** Güncel CPU kullanımını görüntüler.
*   **Mem (Bellek):** RAM ve Swap kullanım detaylarını gösterir.
*   **Disk:** Disk G/Ç (Okuma/Yazma) istatistiklerini raporlar.
*   **Net (Ağ):** Ağ arayüzlerini ve trafik bilgilerini listeler.
*   **Connect (Bağlantılar):** Aktif TCP bağlantılarını gösterir (`netstat` kullanır).
*   **Proc (İşlemler):** Çalışan tüm işlemleri listeler.
*   **Top N (En Çok Kaynak Kullananlar):** CPU kullanımına göre en çok kaynak tüketen ilk N işlemi belirler ve görüntüler.
*   **Report (Raporlama):** Arka planda sistem istatistiklerini düzenli olarak JSON dosyalarına kaydetme görevini başlatır.
*   **Stop-Report (Raporlamayı Durdur):** Arka plandaki raporlama görevini sonlandırır.

## Raporlama Özelliği Hakkında

`report` komutu, istatistikleri düzenli aralıklarla (her 2 dakikada bir) toplayıp projenin bulunduğu dizin altındaki `reports/` klasörüne zaman damgalı JSON dosyaları olarak kaydetmek için bir arka plan işlemi başlatır. Bu işlem `minimon_report.pid` dosyasına kendi PID'sini yazar. `stop-report` komutu, bu PID dosyasını okuyarak arka plan işlemini sonlandırır. Her iki komut da **root yetkisi gerektirir.**
