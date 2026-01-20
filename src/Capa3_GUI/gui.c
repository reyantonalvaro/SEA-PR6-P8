#include <gtk/gtk.h>
#include <stdlib.h> 
#include "gestor.h" 
#include "datos.h" 

#define RUTA_TESTS "../tests/Capa1_tests"

typedef struct {
    TipoEjercicio tipo;
    const char *dialog_id;
    const char *chk_id;
    const char *prefix_1; 
    const char *prefix_2;
    const char *prefix_res;
    const char *fichero_salida;
} GuiConfigGrabacion;

// --- FUNCIONES AUXILIARES ---

static char* abrir_selector_archivos(GtkWidget *widget_origen) {
    GtkWidget *dialog;
    char *filename = NULL;
    GtkWindow *parent_window = NULL;

    // Buscamos la ventana superior que contiene al botón
    if (widget_origen) {
        GtkWidget *toplevel = gtk_widget_get_toplevel(widget_origen);
        if (gtk_widget_is_toplevel(toplevel) && GTK_IS_WINDOW(toplevel)) {
            parent_window = GTK_WINDOW(toplevel);
        }
    }

    dialog = gtk_file_chooser_dialog_new("Seleccionar Fichero de Test",
                                         parent_window,
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         "Cancelar", GTK_RESPONSE_CANCEL,
                                         "Abrir", GTK_RESPONSE_ACCEPT,
                                         NULL);
    
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), RUTA_TESTS);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    }
    
    gtk_widget_destroy(dialog);
    return filename;
}

static void gestionar_grabacion(GtkBuilder *b, GuiConfigGrabacion cfg) {
    GtkWidget *dlg = GTK_WIDGET(gtk_builder_get_object(b, cfg.dialog_id));
    if(!dlg) return;

    gtk_widget_show_all(dlg);
    gint response = gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_hide(dlg);

    if (response != GTK_RESPONSE_REJECT) { 
        double vals1[4], vals2[4], resultados[4];
        char id_widget[64];
        
        GtkWidget *chk = GTK_WIDGET(gtk_builder_get_object(b, cfg.chk_id));
        gboolean auto_calc = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chk));

        for (int i = 0; i < 4; i++) {
            g_snprintf(id_widget, 64, "%s%d", cfg.prefix_1, i+1);
            GtkEntry *e1 = GTK_ENTRY(gtk_builder_get_object(b, id_widget));
            vals1[i] = g_ascii_strtod(gtk_entry_get_text(e1), NULL);

            g_snprintf(id_widget, 64, "%s%d", cfg.prefix_2, i+1);
            GtkEntry *e2 = GTK_ENTRY(gtk_builder_get_object(b, id_widget));
            vals2[i] = g_ascii_strtod(gtk_entry_get_text(e2), NULL);

            if (auto_calc) {
                if (cfg.tipo == EJERCICIO_MOLES) 
                    resultados[i] = calc_moles(vals1[i], vals2[i]);
                else if (cfg.tipo == EJERCICIO_DENSIDAD) 
                    resultados[i] = calc_density(vals1[i], vals2[i]);
                else 
                    resultados[i] = calc_kinetic_energy(vals1[i], vals2[i]);
            } else {
                g_snprintf(id_widget, 64, "%s%d", cfg.prefix_res, i+1);
                GtkEntry *eR = GTK_ENTRY(gtk_builder_get_object(b, id_widget));
                resultados[i] = g_ascii_strtod(gtk_entry_get_text(eR), NULL);
            }
        }
        crear_fichero(cfg.tipo, cfg.fichero_salida, vals1, vals2, resultados, 4);
    }
}

static void gestionar_especifico(GtkBuilder *b, TipoEjercicio tipo, const char *id1, const char *id2) {
    GtkEntry *e1 = GTK_ENTRY(gtk_builder_get_object(b, id1));
    GtkEntry *e2 = GTK_ENTRY(gtk_builder_get_object(b, id2));

    double v1 = g_ascii_strtod(gtk_entry_get_text(e1), NULL);
    double v2 = g_ascii_strtod(gtk_entry_get_text(e2), NULL);

    test_especifico(tipo, v1, v2);
}


// --- CALLBACKS ---

void test_basico_moles(GtkButton *btn, gpointer data) {
    char *f = abrir_selector_archivos(GTK_WIDGET(btn));
    if (f) {
        test_automatico(EJERCICIO_MOLES, f);
        g_free(f);
    }
}
void test_basico_densidad(GtkButton *btn, gpointer data) {
    char *f = abrir_selector_archivos(GTK_WIDGET(btn));
    if (f) {
        test_automatico(EJERCICIO_DENSIDAD, f);
        g_free(f);
    }
}
void test_basico_ecinetica(GtkButton *btn, gpointer data) {
    char *f = abrir_selector_archivos(GTK_WIDGET(btn));
    if (f) {
        test_automatico(EJERCICIO_CINETICA, f);
        g_free(f);
    }
}

void test_especifico_moles(GtkButton *btn, gpointer data) {
    gestionar_especifico(GTK_BUILDER(data), EJERCICIO_MOLES, "entry_Vp", "entry_Vt");
}
void test_especifico_densidad(GtkButton *btn, gpointer data) {
    gestionar_especifico(GTK_BUILDER(data), EJERCICIO_DENSIDAD, "entry_P", "entry_Vt_2");
}
void test_especifico_ecinetica(GtkButton *btn, gpointer data) {
    gestionar_especifico(GTK_BUILDER(data), EJERCICIO_CINETICA, "entry_A", "entry_W");
}

void generar_fichero_moles_clicked(GtkButton *btn, gpointer data) {
    GuiConfigGrabacion cfg = {EJERCICIO_MOLES, "dialogo_moles", "resultado_automatico1", 
                              "entry_Vp", "entry_Vt", "resultado_m", "datos_moles.txt"};
    gestionar_grabacion(GTK_BUILDER(data), cfg);
}
void generar_fichero_densidad_clicked(GtkButton *btn, gpointer data) {
    GuiConfigGrabacion cfg = {EJERCICIO_DENSIDAD, "dialogo_densidad", "resultado_automatico2", 
                              "entry_P", "entry_Vt_d", "resultado_d", "datos_densidad.txt"};
    gestionar_grabacion(GTK_BUILDER(data), cfg);
}
void generar_fichero_ecinetica_clicked(GtkButton *btn, gpointer data) {
    GuiConfigGrabacion cfg = {EJERCICIO_CINETICA, "dialogo_ecinetica", "resultado_automatico3", 
                              "entry_A", "entry_W", "resultado_e", "datos_ecinetica.txt"};
    gestionar_grabacion(GTK_BUILDER(data), cfg);
}

// --- MAIN ---
void finalizar_programa(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

int main(int argc, char **argv) {
    GtkBuilder *builder;
    GtkWidget *ventana;
    GError *error = NULL;

    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    if (!gtk_builder_add_from_file(builder, "interfaz.glade", &error)) {
        g_printerr("Error cargando Glade: %s\n", error->message);
        g_clear_error(&error);
        return 1;
    }

    gtk_builder_connect_signals(builder, builder); 
    ventana = GTK_WIDGET(gtk_builder_get_object(builder, "ventana"));
    g_signal_connect(ventana, "destroy", G_CALLBACK(finalizar_programa), NULL);
    gtk_widget_show_all(ventana);

    gtk_main();
    g_object_unref(builder);
    return 0;
}