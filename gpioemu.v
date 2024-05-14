/* verilator lint_off UNUSED */
/* verilator lint_off MULTIDRIVEN */
/* verilator lint_off WIDTH */
/* verilator lint_off INITIALDLY */
module gpioemu (
    n_reset,                 
    saddress[15:0], srd, swr, 
    sdata_in[31:0], sdata_out[31:0], 
    gpio_in[31:0], gpio_latch,          
    gpio_out[31:0],                     
    clk,                                
    gpio_in_s_insp[31:0]
	
);

input clk;  
input n_reset;      
reg [9:0] A;            // Numer n-tej liczby pierwszej
reg [12:0] W;    // Wynik - N-ta liczba pierwsza
reg [2:0] S;     // Status automatu
reg start;
reg [12:0] divisor;       // Dzielnik liczby - sluzy do sprawdzania czy liczba pierwsza
reg [12:0] number;        // Liczba ktorej pierwszosc sprawdzamy
reg [9:0] counter;        // Sprawdza ktora dana liczba pierwsza jest w kolejnosci
reg [3:0] prime_found = 0;    // Liczba znalezionych liczb pierwszych od wlaczenia systemu

input [15:0] saddress;
input srd;
input swr;
input gpio_latch;
input [31:0] sdata_in;
input [31:0] gpio_in;
output [31:0] gpio_out;
output [31:0] gpio_in_s_insp;
output [31:0] sdata_out;

reg [31:0] sdata_out;
reg [31:0] gpio_in_s;
reg [31:0] gpio_out_s;

// Definicje stanow automatu
localparam INIT = 3'b000;
localparam CHECKING_DIV = 3'b001;
localparam CHECKING_PRIME = 3'b010;
localparam PRIME = 3'b011;
localparam NOT_PRIME = 3'b100;
localparam DONE = 3'b101;
	
//odpowiedz na reset (aktywowane przejsciem 1->0)
always @(negedge n_reset)
begin
    gpio_in_s <= 0;
    gpio_out_s <= 0;
    sdata_out <= 0;
    start <= 0;
    counter <= 0;
    number <= 0;
    divisor <= 0; 
    prime_found <= 0;
    S <= INIT; // Stan poczatkowy: oczekiwanie na zadanie
end

//Obsluga odczytu z GPIO
always @(posedge gpio_latch)
begin
    gpio_in_s <= gpio_in; // Zatrzask w rejestrze wejsciowym
end



// odczyt z gpio
always @(posedge srd)
begin
    // S - 0x104
    if (saddress == 16'h104)
        sdata_out <= S;
    // W - 0xfc
    else if (saddress == 16'hfc) begin
        if(S == DONE) begin
            sdata_out <= (W << 4) + prime_found; //teraz dane sa na [16:4]; na [3:0] liczba znalezionych liczb pierszych od wlaczenia systemu 
        end
        else 
            sdata_out <= 0;
    end
    else
        sdata_out <= sdata_out;
end

// zapis do gpio
always @(posedge swr) begin
    // A - 0xec
    if (saddress == 16'hec) begin
        S <= INIT;
        A <= sdata_in;
        start <= 1;
    end
end



assign gpio_out = gpio_out_s;
assign gpio_in_s_insp = gpio_in_s;

// Proces odpowiedzialny za automatyczne wyznaczanie liczb pierwszych
always @(posedge clk) begin
    case(S)
        INIT: begin
            if (start == 1 && A > 0 && A <= 1000) begin
                counter <= 0;
                number <= 1;
                divisor <= 5;
                S <= CHECKING_DIV; // Przejdz do stanu wyszukiwania
            end
        end
        CHECKING_DIV: begin
            // Sprawdzanie podzielnosci przez 2 i 3 aktualnej liczby
            if (number == 2 || number == 3) begin
                counter <= counter + 1;
                S <= PRIME;
            end else if (number % 3 == 0 || number == 1) begin
                S <= NOT_PRIME;
            end else begin 
                if( (number >> 2) < 5) begin
                    counter <= counter + 1;
                    S <= PRIME;
                end else begin
                    S <= CHECKING_PRIME;
                end
            end
        end
        CHECKING_PRIME: begin
            if (number % divisor == 0) begin
                S <= NOT_PRIME;
            end else begin
                if (divisor * divisor <= number) begin
                    divisor <= divisor + 2;
                end else begin
                    counter <= counter + 1;
                    S <= PRIME;
                end
            end
        end

        PRIME: begin
            //counter <= counter + 1;
            if (counter == A) begin
                prime_found <= prime_found + 1;
                S <= DONE;
            end else begin
                if (number == 2) begin
                    number <= number + 1;
                end else begin 
                    number <= number + 2;
                end 
                divisor <= 5;
                S <= CHECKING_DIV;
            end   
        end
        NOT_PRIME: begin
            if (number == 1) begin
                number <= number + 1;
            end else begin 
                number <= number + 2;
            end    
            divisor <= 5;
            S <= CHECKING_DIV;
        end
        DONE: begin
            W <= number;
            start <= 0;
        end

        default: S <= INIT;
    endcase
end
endmodule